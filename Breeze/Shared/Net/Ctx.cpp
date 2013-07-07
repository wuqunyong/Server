/*
 Copyright (c) 2007-2011 iMatix Corporation
 Copyright (c) 2007-2011 Other contributors as noted in the AUTHORS file

 This file is part of 0MQ.

 0MQ is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 0MQ is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Ctx.h"

#include <new>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#else
#include <unistd.h>
#endif

#include "IOThread.h"
#include "MySQLRPCClient.h"
#include "MySQLRPCServer.h"
#include "SocketBase.h"
#include "ZmqEngine.h"

#include "../Utils/Guard.h"

#define MAX_SOCKETS (512)

FREEZE_NET::Ctx::Ctx(uint32_t io_threads) :
	tag_(0xbadcafe0), terminating_(false)
{
	this->gen_serial_num_.Add(1);

	//  Initialise the array of mailboxes. Additional three slots are for
	//  internal log socket and the zmq_term thread the reaper thread.
	slot_count_ = MAX_SOCKETS + io_threads + enum_count;
	slots_ = (Mailbox**) malloc(sizeof(Mailbox*) * slot_count_);
	assert(slots_ != NULL);

	//  Initialise the infrastructure for zmq_term thread.
	slots_[term_tid] = &term_mailbox_;

	mysql_rpc_client_ = new (std::nothrow) MySQLRPCClient(this, rpc_mysql_client_tid);
	assert(mysql_rpc_client_ != NULL);
	slots_[rpc_mysql_client_tid] = mysql_rpc_client_->GetMailbox();
	mysql_rpc_client_->Start();

	mysql_rpc_server_ = new (std::nothrow) MySQLRPCServer(this, rpc_mysql_server_tid);
	assert(mysql_rpc_server_ != NULL);
	slots_[rpc_mysql_server_tid] = mysql_rpc_server_->GetMailbox();
	mysql_rpc_server_->Start();

	mysql_rpc_log_ = new (std::nothrow) MySQLRPCServer(this, rpc_mysql_log_tid);
	assert(mysql_rpc_log_ != NULL);
	slots_[rpc_mysql_log_tid] = mysql_rpc_log_->GetMailbox();
	mysql_rpc_log_->Start();

	//  Create I/O thread objects and launch them.
	for (uint32_t i = enum_count; i != io_threads + enum_count; i++)
	{
		IOThread *io_thread = new (std::nothrow) IOThread(this, i);
		assert(io_thread != NULL);
		io_threads_.push_back(io_thread);
		slots_[i] = io_thread->GetMailbox();
		io_thread->Start();
	}

	//  In the unused part of the slot array, create a list of empty slots.
	for (int32_t i = (int32_t) slot_count_ - 1; i >= (int32_t) io_threads + enum_count; i--)
	{
		empty_slots_.push_back(i);
		slots_[i] = NULL;
	}

	IOThread *io_thread = ChooseIOThread(0);
	assert(NULL != io_thread);

	this->container_ptr = new (std::nothrow) SocketBase(io_thread);
	assert(NULL != this->container_ptr);
}

bool FREEZE_NET::Ctx::CheckTag()
{
	return tag_ == 0xbadcafe0;
}

FREEZE_NET::Ctx::~Ctx()
{
	this->container_ptr->Stop();


	//  Remove the tag, so that the object is considered dead.
	//  Ask I/O threads to terminate. If stop signal wasn't sent to I/O
	//  thread subsequent invocation of destructor would hang-up.
	for (io_threads_t::size_type i = 0; i != io_threads_.size (); i++)
	{
		io_threads_[i]->Stop();
	}


	//  Wait till I/O threads actually terminate.
	for (io_threads_t::size_type i = 0; i != io_threads_.size (); i++)
	{
		delete io_threads_[i];
	}

	if (mysql_rpc_client_ != NULL)
	{
		mysql_rpc_client_->Stop();
		delete mysql_rpc_client_;
		mysql_rpc_client_ = NULL;
	}

	if (mysql_rpc_server_ != NULL)
	{
		mysql_rpc_server_->Stop();
		delete mysql_rpc_server_;
		mysql_rpc_server_ = NULL;
	}

	if (mysql_rpc_log_ != NULL)
	{
		mysql_rpc_log_->Stop();
		delete mysql_rpc_log_;
		mysql_rpc_log_ = NULL;
	}

	//  Deallocate the array of mailboxes. No special work is
	//  needed as mailboxes themselves were deallocated with their
	//  corresponding io_thread/socket objects.
	free(slots_);

	tag_ = 0xdeadbeef;
}

int FREEZE_NET::Ctx::Terminate()
{
	terminating_ = true;
	delete this;
	return 0;
}

void FREEZE_NET::Ctx::SendCommand(uint32_t tid, const Command& command)
{
	slots_[tid]->Send(command);
}

FREEZE_NET::IOThread *FREEZE_NET::Ctx::ChooseIOThread(uint64_t affinity)
{
	if (io_threads_.empty())
	{
		return NULL;
	}

	//  Find the I/O thread with minimum load.
	int min_load = -1;
	io_threads_t::size_type result = 0;
	for (io_threads_t::size_type i = 0; i != io_threads_.size(); i++)
	{
		int load = io_threads_[i]->GetLoad();
		if (min_load == -1 || load < min_load)
		{
			min_load = load;
			result = i;
		}
	}
	assert(min_load != -1);
	return io_threads_[result];
}

FREEZE_NET::MySQLRPCClient* FREEZE_NET::Ctx::MysqlRPCClient()
{
	return mysql_rpc_client_;
}

FREEZE_NET::SocketBase* FREEZE_NET::Ctx::GetSocketBase()
{
	return container_ptr;
}


FREEZE_NET::MySQLRPCServer* FREEZE_NET::Ctx::MysqlRPCServer()
{
	return mysql_rpc_server_;
}

FREEZE_NET::MySQLRPCServer* FREEZE_NET::Ctx::MysqlRPCLog()
{
	return mysql_rpc_log_;
}

uint32_t FREEZE_NET::Ctx::GenSerialNum(void)
{
	return this->gen_serial_num_.Add(1);
}

void FREEZE_NET::Ctx::RegisterZmqEngine(uint32_t serial_num, class ZmqEngine* ptr_engine)
{
	Guard guard(this->engine_mutex_);

	std::pair<std::map<uint32_t, class ZmqEngine*>::iterator,bool> ret;
	ret = engine_.insert ( std::pair<uint32_t, class ZmqEngine*>(serial_num,ptr_engine) );
	if (ret.second == false) 
	{
		std::cout << "ZmqEngine serial_num : " << ret.first->first << std::endl;
	}

	std::map<uint32_t, std::string>::iterator ite = send_buf_.find(serial_num);
	if (ite == send_buf_.end())
	{
		std::string empty;
		send_buf_[serial_num] = empty;
	}
}

void FREEZE_NET::Ctx::RegisterSendBuf(uint32_t serial_num)
{
	Guard guard(this->engine_mutex_);

	std::string empty;
	send_buf_[serial_num] = empty;
}

void FREEZE_NET::Ctx::UnregisterZmqEngine(uint32_t serial_num)
{
	Guard guard(this->engine_mutex_);

	engine_.erase(serial_num);
	send_buf_.erase(serial_num);
}

bool  FREEZE_NET::Ctx::SendBuf(uint32_t serial_num, std::string buf, ProtocolHead protocol_haad)
{
	Guard guard(this->engine_mutex_);

	std::map<uint32_t, std::string>::iterator ite = send_buf_.find(serial_num);
	if (ite != send_buf_.end())
	{
		protocol_haad.total_length_ = sizeof(ProtocolHead) + buf.length();

		ByteBuffer temp_buffer;
		temp_buffer << protocol_haad;

		std::string temp_str((const char*)temp_buffer.RdPos(), temp_buffer.Length());
		ite->second += temp_str;

		ite->second += buf;
		return true;

		//this->ActivateSendBuf(serial_num);
	}
	else
	{
		return false;
	}
}

bool FREEZE_NET::Ctx::SendBuf(uint32_t serial_num, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad)
{
	Guard guard(this->engine_mutex_);

	std::map<uint32_t, std::string>::iterator ite = send_buf_.find(serial_num);
	if (ite != send_buf_.end())
	{
		protocol_haad.total_length_ = sizeof(ProtocolHead) + ptr_size;

		ByteBuffer temp_buffer;
		temp_buffer << protocol_haad;

		std::string temp_str((const char*)temp_buffer.RdPos(), temp_buffer.Length());
		ite->second += temp_str;

		std::string read_str((const char*)ptr, ptr_size);
		ite->second += read_str;

		return true;
		//this->ActivateSendBuf(serial_num);
	}
	else
	{
		return false;
	}
}

std::string FREEZE_NET::Ctx::BuildPackage(ProtocolHead protocol_haad, unsigned char* ptr, uint32_t ptr_size)
{
	protocol_haad.total_length_ = sizeof(ProtocolHead) + ptr_size;

	ByteBuffer temp_buffer;
	temp_buffer << protocol_haad;

	std::string package_buffer;

	std::string temp_str((const char*)temp_buffer.RdPos(), temp_buffer.Length());
	package_buffer += temp_str;

	std::string read_str((const char*)ptr, ptr_size);
	package_buffer += read_str;

	return package_buffer;
}

//void FREEZE_NET::Ctx::ActivateSendBuf(uint32_t serial_num)
//{
//	std::map<uint32_t, class ZmqEngine*>::iterator ite = engine_.find(serial_num);
//	if (ite != engine_.end())
//	{
//		ite->second->TriggerActivateOut();
//	}
//}

std::string FREEZE_NET::Ctx::DrainBuf(uint32_t serial_num)
{
	Guard guard(this->engine_mutex_);

	std::map<uint32_t, std::string>::iterator ite = send_buf_.find(serial_num);
	if (ite != send_buf_.end())
	{
		std::string duplicate_string = ite->second;
		ite->second.clear();

		return duplicate_string;
	}
	else
	{
		std::string empty;
		return empty;
	}
}

void FREEZE_NET::Ctx::Close(uint32_t serial_num)
{
	Guard guard(this->engine_mutex_);

	std::map<uint32_t, class ZmqEngine*>::iterator ite = engine_.find(serial_num);
	if (ite != engine_.end())
	{
		ite->second->Terminate();
	}	
}

Opcodes& FREEZE_NET::Ctx::GetOpcodes(void)
{
	return this->opcodes_;
}

SpinLock& FREEZE_NET::Ctx::GetEngineMutex()
{
	return this->engine_mutex_;
}

std::map<uint32_t, FREEZE_NET::ZmqEngine*>& FREEZE_NET::Ctx::GetEngine()
{
	return this->engine_;
}