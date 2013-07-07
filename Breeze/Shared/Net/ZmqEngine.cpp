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

#include "Fd.h"

#include <string.h>
#include <new>
#include <assert.h>
#include <iostream>
#include <exception>

#ifndef WIN32
#include <unistd.h>
#endif

#include "ZmqEngine.h"
#include "IOThread.h"
#include "Object.h"
#include "Ctx.h"
#include "MySQLRPCClient.h"

FREEZE_NET::ZmqEngine::ZmqEngine (IOThread *io_thread, fd_t fd, uint32_t serial_num) :
	Own(io_thread),
	io_thread_(io_thread),
    plugged_(false),
	serial_num_(serial_num)
{
    //  Initialise the underlying socket.
    int rc = tcp_socket_.Open(fd);
    assert(rc == 0);
}

FREEZE_NET::ZmqEngine::~ZmqEngine ()
{
    assert(!plugged_);
}

void  FREEZE_NET::ZmqEngine::ProcessPlug()
{
	Plug(io_thread_);
}


void FREEZE_NET::ZmqEngine::ProcessActivateOut()
{
	ActivateOut();
}

void FREEZE_NET::ZmqEngine::ProcessTerm(int linger)
{
	Unplug();
	Own::ProcessTerm(linger);
}


void FREEZE_NET::ZmqEngine::Plug(IOThread *io_thread)
{
    assert(!plugged_);
    plugged_ = true;


    //  Connect to I/O threads poller object.
    IOObject::Plug(io_thread);
    handle_ = AddFd(tcp_socket_.GetFd());
    SetPollin(handle_);
    SetPollout(handle_);

	GetCtx()->RegisterZmqEngine(this->serial_num_, this);

    //  Flush all the data that may have been already received downstream.
    InEvent();

}

void FREEZE_NET::ZmqEngine::Unplug()
{
    assert(plugged_);
    plugged_ = false;

	GetCtx()->UnregisterZmqEngine(this->serial_num_);
    //  Cancel all fd subscriptions.
    RmFd(handle_);

    //  Disconnect from I/O threads poller object.
    IOObject::Unplug();

	ByteBuffer *ptr_packet_buffer = new ByteBuffer();

	ProtocolHead protocol_head;
	protocol_head.opcode_ = Opcodes::CLOSE_SESSION;
	protocol_head.total_length_ = sizeof(ProtocolHead);
	protocol_head.cur_session_serial_num_ = this->serial_num_;
	protocol_head.suspend_session_serial_num_ = this->serial_num_;

	(*ptr_packet_buffer) << protocol_head;

	GetCtx()->MysqlRPCClient()->Push(this->serial_num_, ptr_packet_buffer);
}

void FREEZE_NET::ZmqEngine::Terminate()
{
    //Unplug();
	if (GetOwner() != NULL)
	{
		SendTermReq(GetOwner(), this);
	}
}

void FREEZE_NET::ZmqEngine::InEvent()
{
	while(1)
	{
		bool disconnection = false;

		//  If there's no data to process in the buffer...
		char buf[4096] = {0};

		//  Retrieve the buffer and read as much data as possible.
		size_t read_size = tcp_socket_.Read(buf, 4096);

		//  Check whether the peer has closed the connection.
		if (read_size == (size_t) -1) 
		{
			disconnection = true;
			Error();
		}
		else
		{
			try
			{
				if (read_size > 0)
				{		
					//  Push the data to the decoder.
					decoder_.Append<char>(buf, read_size);

					uint32_t packet_len = 0;
					decoder_.Peek<uint32_t>(packet_len);

					while (decoder_.Length() >= packet_len)
					{
						//先读取包头，修正当前通信SESSION
						ProtocolHead read_head;
						bool result = decoder_.Peek<ProtocolHead>(read_head);
						if (result)
						{
							//路由协议
							if (Opcodes::ROUTING == read_head.opcode_)
							{
								char* packet_ptr = decoder_.Drain(packet_len);
								ByteBuffer* ptr_packet_buffer = new ByteBuffer((const unsigned char*)packet_ptr, packet_len);

								//读取路由包头，将其丢弃，将原数据包投递给消息队列，
								//并将suspend_session_serial_num_ 设为原数据cur_session_serial_num_
								//在该SESSION里新创建的映射


								ProtocolHead routing_head;

								//丢弃路由包头
								(*ptr_packet_buffer) >> routing_head;

								//获取原始包长度
								uint32_t original_packet_len = 0;
								(*ptr_packet_buffer).Peek<uint32_t>(original_packet_len);
								if ((*ptr_packet_buffer).Length() == original_packet_len)
								{
									ProtocolHead original_head;
									bool original_result = (*ptr_packet_buffer).Peek<ProtocolHead>(original_head);
									if (original_result)
									{
										//原始包的当前通信SESSION
										uint32_t original_serial_num = original_head.cur_session_serial_num_;

										original_head.cur_session_serial_num_ = this->serial_num_;

										//相邻节点对一条连接的映射
										original_head.suspend_session_serial_num_ = this->FindRoutingNum(original_serial_num);


										char* packet_ptr = (*ptr_packet_buffer).Drain(original_packet_len);
										*((ProtocolHead*)packet_ptr) = original_head;

										ByteBuffer* ptr_original = new ByteBuffer((const unsigned char*)packet_ptr, original_packet_len);

										GetCtx()->MysqlRPCClient()->Push(original_head.suspend_session_serial_num_, ptr_original);

									} 
									else
									{
										std::cout << "ROUTING error !" << __FILE__ << " " << __LINE__ << std::endl;
									}

									delete ptr_packet_buffer;
								}
								else
								{
									std::cout << "ROUTING error ! " << __FILE__ << " " << __LINE__ << std::endl;
									delete ptr_packet_buffer;
								}
							} 
							else
							{
								read_head.cur_session_serial_num_ = this->serial_num_;
								read_head.suspend_session_serial_num_ = this->serial_num_;

								char* packet_ptr = decoder_.Drain(packet_len);
								*((ProtocolHead*)packet_ptr) = read_head;

								ByteBuffer* ptr_packet_buffer = new ByteBuffer((const unsigned char*)packet_ptr, packet_len);

								GetCtx()->MysqlRPCClient()->Push(this->serial_num_, ptr_packet_buffer);
							}

							//重新获得包头长度
							bool result = decoder_.Peek<uint32_t>(packet_len);
							if (!result)
							{
								break;
							}
						}
						else
						{
							disconnection = true;
							Error();

							break;
						}
					}

				}
			}
			catch(std::exception& e)
			{
				std::cout << "catch exception " << e.what() << __FILE__ << " " << __LINE__ << std::endl;
				disconnection = true;
				Error();
			}
			catch(...)
			{
				std::cout << "catch ... " << __FILE__ << " " << __LINE__ << std::endl;
				disconnection = true;
				Error();
			}
		}

		if (read_size != 4096)
		{
			break;
		}
		
	}
}

void FREEZE_NET::ZmqEngine::OutEvent()
{
    if (encoder_.Length()) 
	{
		//  If there are any data to write in write buffer, write as much as
		//  possible to the socket.
		int nbytes = tcp_socket_.Write(encoder_.RdPos(), encoder_.Length());

		//  Handle problems with the connection.
		if (nbytes == -1) 
		{
			Error();
			return;
		}

		encoder_.AddRdPos(nbytes);
    }

	std::string read_buf = GetCtx()->DrainBuf(this->serial_num_);
	if (!read_buf.empty())
	{
		encoder_.Append<char>((char* )read_buf.c_str(), read_buf.length());
	}
	
	//if (encoder_.Length() == 0)
	//{
	//	ResetPollout(handle_);
	//}
	
}

void FREEZE_NET::ZmqEngine::ActivateOut()
{
    SetPollout(handle_);

    //  Speculative write: The assumption is that at the moment new message
    //  was sent by the user the socket is probably available for writing.
    //  Thus we try to write the data to socket avoiding polling for POLLOUT.
    //  Consequently, the latency should be better in request/reply scenarios.
    OutEvent();
}

void FREEZE_NET::ZmqEngine::ActivateIn()
{
    SetPollin(handle_);

    //  Speculative read.
    InEvent();
}

void FREEZE_NET::ZmqEngine::Error()
{
    //Unplug();

    //delete this;
	if (GetOwner() != NULL)
	{
		SendTermReq(GetOwner(), this);
	}
}

uint32_t FREEZE_NET::ZmqEngine::FindRoutingNum(uint32_t serial_num)
{
	std::map<uint32_t, uint32_t>::iterator ite = routing_serial_num_.find(serial_num);
	if (ite == routing_serial_num_.end())
	{
		uint32_t routing_num = GetCtx()->GenSerialNum();
		routing_serial_num_[serial_num] = routing_num;

		return routing_num;
	} 
	else
	{
		return ite->second;
	}
	
}