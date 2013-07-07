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

#include <assert.h>
#include <new>

#include "Fd.h"
#include "MySQLRPCClient.h"
#include "Ctx.h"
#include "Command.h"
#include "MySQLRPCServer.h"

#include "../Utils/Timer.h"

#ifndef WIN32
#include <unistd.h>
#endif

#include "../Utils/CommandBase.h"

FREEZE_NET::SessionMsgQueue::SessionMsgQueue()
{
	this->serial_num_ = 0;
	this->state_ = RUNNING;
}

FREEZE_NET::SessionMsgQueue::~SessionMsgQueue()
{
	if (!msg_queue_.empty())
	{
		std::cout << "SessionMsgQueue size: " << msg_queue_.size() << std::endl; 
	}
	
	while(!msg_queue_.empty())
	{
		ByteBuffer * ptr_buff = msg_queue_.front();
		msg_queue_.pop();
		delete ptr_buff;
		ptr_buff = NULL;
	}
}

void FREEZE_NET::SessionMsgQueue::SetSerialNum(uint32_t serial_num)
{
	this->serial_num_ = serial_num;
}

uint32_t FREEZE_NET::SessionMsgQueue::GetSerialNum(void)
{
	return this->serial_num_;
}


void FREEZE_NET::SessionMsgQueue::SetState(uint32_t val)
{
	this->state_ = val;
}

uint32_t FREEZE_NET::SessionMsgQueue::GetState(void)
{
	return this->state_;
}

std::queue<ByteBuffer *>& FREEZE_NET::SessionMsgQueue::GetMsgQueue()
{
	return this->msg_queue_;
}

FREEZE_NET::MySQLRPCClient::MySQLRPCClient(Ctx* ctx, uint32_t tid) :
	Object(ctx, tid)
{
	producer_ = new (std::nothrow) std::queue<uint32_t>;
	assert(producer_ != NULL);

	consumer_ = new (std::nothrow) std::queue<uint32_t>;
	assert(consumer_);

	poller_ = new (std::nothrow) Poller;
	assert(poller_ != NULL);

	mailbox_handle_ = poller_->AddFd(mailbox_.GetFd(), this);
	poller_->SetPollin(mailbox_handle_);
	poller_->SetPollout(mailbox_handle_);
}

FREEZE_NET::MySQLRPCClient::~MySQLRPCClient()
{
	delete poller_;

	delete producer_;
	producer_ = NULL;

	delete consumer_;
	consumer_ = NULL;
}

void FREEZE_NET::MySQLRPCClient::Start()
{
	//  Start the underlying I/O thread.
	poller_->Start();
}

void FREEZE_NET::MySQLRPCClient::Stop()
{
	SendStop();
}

FREEZE_NET::Mailbox *FREEZE_NET::MySQLRPCClient::GetMailbox()
{
	return &mailbox_;
}

int FREEZE_NET::MySQLRPCClient::GetLoad()
{
	return poller_->GetLoad();
}

void FREEZE_NET::MySQLRPCClient::InEvent()
{
	//  TODO: Do we want to limit number of commands I/O thread can
	//  process in a single go?

	while (true)
	{
		//  Get the next command. If there is none, exit.
		Command cmd;
		int rc = mailbox_.Recv(&cmd);
		if (rc != 0 && errno == EINTR)
			continue;
		if (rc != 0 && errno == EAGAIN)
			break;
		assert(rc == 0);

		//  Process the command.
		cmd.destination_->ProcessCommand(cmd);
	}
}

void FREEZE_NET::MySQLRPCClient::OutEvent()
{
	uint64_t begin_time = Timer::GetMilliSeconds(); 

	uint64_t packet_count = 0;

	while(!this->consumer_->empty())
	{
		uint32_t temp_serial_num = this->consumer_->front();
		this->consumer_->pop();
		packet_count++;

		uint32_t temp_state = GetSessionState(temp_serial_num);
		switch(temp_state)
		{
		case SessionMsgQueue::RUNNING:
			{
				ByteBuffer* temp = this->Pop(temp_serial_num);
				if (temp != NULL)
				{
					ProtocolHead protocol_head;
					bool result = temp->Peek<ProtocolHead>(protocol_head);

					if (result)
					{
						IDelegate* ptr_delegate = GetCtx()->GetOpcodes().FindHandler(protocol_head.opcode_);

						if (ptr_delegate != NULL)
						{
							try
							{
								ptr_delegate->Invoke(temp);
							}
							catch(std::exception& e)
							{
								std::cout << "catch exception " << e.what() << __FILE__ << " " << __LINE__ << std::endl;
								GetCtx()->Close(protocol_head.suspend_session_serial_num_);
							}
							catch(...)
							{
								std::cout << "catch ... " << __FILE__ << " " << __LINE__ << std::endl;
								GetCtx()->Close(protocol_head.suspend_session_serial_num_);
							}

						}
						else
						{
							std::cout << "protocol_head.opcode_ : " << protocol_head.opcode_ << " not register !" << std::endl;
						}
					} 
					else
					{
						std::cout << "protocol_head peak error!" << std::endl; 
					}
					delete temp;
					temp = NULL;

					switch(protocol_head.opcode_)
					{
					case Opcodes::CLOSE_SESSION:
						{
							this->DestroySession(protocol_head.suspend_session_serial_num_);
							break;
						}
					}
				}
				break;
			}
		case SessionMsgQueue::BLOCKED:
			{
				this->TransferPush(temp_serial_num);
				break;
			}
		case SessionMsgQueue::ENUM_COUNT:
			{
				std::cout << "SessionMsgQueue::ENUM_COUNT " << __FILE__ << " " << __LINE__ << std::endl;
				break;
			}
		default:
			{
				assert(0);
				break;
			}
		}

	}

	this->Swap();
	uint64_t end_time = Timer::GetMilliSeconds();

	uint64_t interval_value = end_time - begin_time;

	GetCtx()->GetOpcodes().Update();

	if (interval_value < 50)
	{
#ifdef WIN32
		Sleep(50 - interval_value);
#else
		usleep((50 - interval_value)*1000);
#endif
	}

}

void FREEZE_NET::MySQLRPCClient::TimerEvent(int id)
{
	//  No timers here. This function is never called.
	assert(false);
}

FREEZE_NET::Poller *FREEZE_NET::MySQLRPCClient::GetPoller()
{
	assert(poller_ != NULL);
	return poller_;
}

void FREEZE_NET::MySQLRPCClient::ProcessStop()
{
	poller_->RmFd(mailbox_handle_);
	poller_->Stop();
}

void FREEZE_NET::MySQLRPCClient::SendRPC(char const* query_statement,
	CommandBaseEx* callback_cmd,
	uint32_t serial_num,
	uint32_t set_session_state,
	char const* table)
{
	assert(NULL != callback_cmd);
	assert(set_session_state < SessionMsgQueue::ENUM_COUNT);

	//调用RPC是阻塞该SESSION的消息队列
	SetSessionState(serial_num, SessionMsgQueue::BLOCKED);

	uint32_t msg_id = msg_id_.Add(1);

	MySQLRPCServer* destination = GetCtx()->MysqlRPCServer();
	assert(destination != NULL);

	ResultSet* query_result = NULL;

	Command cmd;
	cmd.destination_ = destination;
	cmd.type_ = Command::RPC_MYSQL_REQUEST;

	cmd.args_.RPC_MYSQL_REQUEST.msg_id_ = msg_id;

	std::size_t ptr_table_size = strlen(table) + 1;
	char* ptr_table = new char[ptr_table_size];
	memset(ptr_table, 0, ptr_table_size);
	memcpy(ptr_table, table, ptr_table_size);

	std::size_t ptr_query_statement_size = strlen(query_statement) + 1;
	char* ptr_query_statement = new char[ptr_query_statement_size];
	memset(ptr_query_statement, 0, ptr_query_statement_size);
	memcpy(ptr_query_statement, query_statement, ptr_query_statement_size);

	cmd.args_.RPC_MYSQL_REQUEST.table_ = ptr_table;
	cmd.args_.RPC_MYSQL_REQUEST.query_statement_ = ptr_query_statement;
	cmd.args_.RPC_MYSQL_REQUEST.callback_cmd_ = callback_cmd;
	cmd.args_.RPC_MYSQL_REQUEST.query_result_ = query_result;
	cmd.args_.RPC_MYSQL_REQUEST.request_object_ = this;
	cmd.args_.RPC_MYSQL_REQUEST.serial_num_ = serial_num;
	cmd.args_.RPC_MYSQL_REQUEST.set_session_state_ = set_session_state;

	AddStubs(msg_id, cmd);

	SendRPCMysqlRequest(destination, cmd.args_.RPC_MYSQL_REQUEST.msg_id_,
			cmd.args_.RPC_MYSQL_REQUEST.table_,
			cmd.args_.RPC_MYSQL_REQUEST.query_statement_,
			cmd.args_.RPC_MYSQL_REQUEST.callback_cmd_,
			cmd.args_.RPC_MYSQL_REQUEST.query_result_,
			cmd.args_.RPC_MYSQL_REQUEST.request_object_);

}

void FREEZE_NET::MySQLRPCClient::SendRPCNotify(char const* query_statement, char const* table)
{
	MySQLRPCServer* destination = GetCtx()->MysqlRPCServer();
	assert(NULL != destination);

	std::size_t ptr_table_size = strlen(table) + 1;
	char* ptr_table = new char[ptr_table_size];
	memset(ptr_table, 0, ptr_table_size);
	memcpy(ptr_table, table, ptr_table_size);

	std::size_t ptr_query_statement_size = strlen(query_statement) + 1;
	char* ptr_query_statement = new char[ptr_query_statement_size];
	memset(ptr_query_statement, 0, ptr_query_statement_size);
	memcpy(ptr_query_statement, query_statement, ptr_query_statement_size);

	SendRPCMysqlNotify(destination, ptr_query_statement, ptr_table);
}

void FREEZE_NET::MySQLRPCClient::SendRPCNotifyLog(char const* query_statement, char const* table)
{
	MySQLRPCServer* destination = GetCtx()->MysqlRPCLog();
	assert(NULL != destination);

	std::size_t ptr_table_size = strlen(table) + 1;
	char* ptr_table = new char[ptr_table_size];
	memset(ptr_table, 0, ptr_table_size);
	memcpy(ptr_table, table, ptr_table_size);

	std::size_t ptr_query_statement_size = strlen(query_statement) + 1;
	char* ptr_query_statement = new char[ptr_query_statement_size];
	memset(ptr_query_statement, 0, ptr_query_statement_size);
	memcpy(ptr_query_statement, query_statement, ptr_query_statement_size);

	SendRPCMysqlNotify(destination, ptr_query_statement, ptr_table);
}

void FREEZE_NET::MySQLRPCClient::ProcessRPCMysqlResponse(uint32_t msg_id,
		uint8_t return_result, ResultSet* query_result)
{
	Command cmd;

	bool result = FindStubs(msg_id, cmd);
	if (!result)
	{
		std::cout << "mysql rpc response mapped error !" << std::endl;
	}
	else
	{
		if (cmd.type_ == Command::RPC_MYSQL_REQUEST)
		{	
			//收到PRC回复时，重新设置该SESSION，为预先设置的的状态
			SetSessionState(cmd.args_.RPC_MYSQL_REQUEST.serial_num_, cmd.args_.RPC_MYSQL_REQUEST.set_session_state_);
		}

		if (return_result == 0)
		{
			std::cout << "mysql rpc query statement execute error !"
					<< std::endl;
		}

		switch (cmd.type_)
		{
			case Command::RPC_MYSQL_REQUEST:
			{
				if (cmd.args_.RPC_MYSQL_REQUEST.callback_cmd_ != NULL)
				{
					try
					{
						cmd.args_.RPC_MYSQL_REQUEST.callback_cmd_->Run(query_result);
					}
					catch(std::exception& e)
					{
						std::cout << "catch exception " << e.what() << __FILE__ << " " << __LINE__ << std::endl;
					}
					catch(...)
					{
						std::cout << "catch ... " << __FILE__ << " " << __LINE__ << std::endl;
					}		
				}
				else
				{
					//std::cout << "ite->second.args_.RPC_MYSQL_REQUEST.callback_cmd_ == NULL" << std::endl;
				}
				break;
			}
			default:
			{
				std::cout << "mysql rpc request tyep error !" << std::endl;
				break;
			}
		}

		if (cmd.args_.RPC_MYSQL_REQUEST.table_ != NULL)
		{
			delete[] cmd.args_.RPC_MYSQL_REQUEST.table_;
			cmd.args_.RPC_MYSQL_REQUEST.table_ = NULL;
		}
		if (cmd.args_.RPC_MYSQL_REQUEST.query_statement_ != NULL)
		{
			delete[] cmd.args_.RPC_MYSQL_REQUEST.query_statement_;
			cmd.args_.RPC_MYSQL_REQUEST.query_statement_ = NULL;
		}

		if (query_result)
		{
			delete query_result;
			query_result = NULL;
		}
	

		RemoveStubs(msg_id);
	}
}

void FREEZE_NET::MySQLRPCClient::AddStubs(uint32_t msg_id, Command& cmd)
{
	Guard guard(this->lock_);
	client_stubs_[msg_id] = cmd;
}

bool FREEZE_NET::MySQLRPCClient::FindStubs(uint32_t msg_id, Command& cmd)
{
	Guard guard(this->lock_);
	std::map<uint32_t, Command>::iterator ite = client_stubs_.find(msg_id);
	if (ite == client_stubs_.end())
	{
		return false;
	}
	else
	{
		cmd = ite->second;
		return true;
	}
}

void FREEZE_NET::MySQLRPCClient::RemoveStubs(uint32_t msg_id)
{
	Guard guard(this->lock_);
	client_stubs_.erase(msg_id);
}

void FREEZE_NET::MySQLRPCClient::HandlePRCResponse(ResultSet* query_result)
{
	if (NULL != query_result)
	{
		while (query_result->MoveNext())
		{
			uint32_t field_1 = 0;
			uint32_t field_2 = 0;
			uint32_t field_3 = 0;
			uint32_t field_4 = 0;
			uint32_t field_5 = 0;
			std::string field_6;

			if ((*query_result >> field_1) && (*query_result >> field_2)
					&& (*query_result >> field_3) && (*query_result >> field_4)
					&& (*query_result >> field_5) && (*query_result >> field_6))
			{
				std::cout << "field_1 : " << field_1 << std::endl;
				std::cout << "field_2 : " << field_2 << std::endl;
				std::cout << "field_3 : " << field_3 << std::endl;
				std::cout << "field_4 : " << field_4 << std::endl;
				std::cout << "field_5 : " << field_5 << std::endl;
				std::cout << "field_6 : " << field_6 << std::endl;

			}
		}
	}
}

void FREEZE_NET::MySQLRPCClient::Swap(void)
{
	Guard guard(this->lock_);

	if (!this->consumer_->empty())
	{
		return;
	}

	std::queue<uint32_t> *temp = this->consumer_;
	this->consumer_ = this->producer_;
	this->producer_ = temp;
}

void FREEZE_NET::MySQLRPCClient::Push(uint32_t serial_num, ByteBuffer* ptr_value)
{
	Guard guard(this->lock_);

	this->producer_->push(serial_num);

	std::map<uint32_t, SessionMsgQueue>::iterator ite = session_map_.find(serial_num);
	if (ite == session_map_.end())
	{
		SessionMsgQueue msg_queue;
		msg_queue.SetSerialNum(serial_num);

		session_map_[msg_queue.GetSerialNum()] = msg_queue;
		session_map_[msg_queue.GetSerialNum()].GetMsgQueue().push(ptr_value);
	} 
	else
	{
		ite->second.GetMsgQueue().push(ptr_value);
	}
	
}

ByteBuffer* FREEZE_NET::MySQLRPCClient::Pop(uint32_t serial_num)
{
	Guard guard(this->lock_);

	std::map<uint32_t, SessionMsgQueue>::iterator ite = session_map_.find(serial_num);
	if (ite == session_map_.end())
	{
		return NULL;
	} 
	else
	{
		if (!ite->second.GetMsgQueue().empty())
		{
			ByteBuffer* temp = ite->second.GetMsgQueue().front();
			ite->second.GetMsgQueue().pop();
			return temp;
		}
		else
		{
			return NULL;
		}
	}
}

void FREEZE_NET::MySQLRPCClient::TransferPush(uint32_t serial_num)
{
	Guard guard(this->lock_);

	this->producer_->push(serial_num);
}

void FREEZE_NET::MySQLRPCClient::DestroySession(uint32_t serial_num)
{
	Guard guard(this->lock_);

	std::map<uint32_t, SessionMsgQueue>::iterator ite = session_map_.find(serial_num);
	if (ite != session_map_.end())
	{
		session_map_.erase(ite);
	}
}

uint32_t FREEZE_NET::MySQLRPCClient::GetSessionState(uint32_t serial_num)
{
	Guard guard(this->lock_);

	std::map<uint32_t, SessionMsgQueue>::iterator ite = session_map_.find(serial_num);
	if (ite != session_map_.end())
	{
		return ite->second.GetState();
	} 
	else
	{
		return SessionMsgQueue::ENUM_COUNT;
	}
}

void FREEZE_NET::MySQLRPCClient::SetSessionState(uint32_t serial_num, uint32_t state)
{
	assert(state < SessionMsgQueue::ENUM_COUNT);

	Guard guard(this->lock_);

	std::map<uint32_t, SessionMsgQueue>::iterator ite = session_map_.find(serial_num);
	if (ite != session_map_.end())
	{
		ite->second.SetState(state);
	} 
}

