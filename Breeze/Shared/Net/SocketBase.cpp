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

#include <new>
#include <string>
#include <algorithm>

#include "Fd.h"

#if defined WIN32
#else
#include <unistd.h>
#endif

#include <assert.h>

#include "SocketBase.h"
#include "ZmqListener.h"
#include "ZmqConnecter.h"
#include "IOThread.h"
#include "Clock.h"
#include "Ctx.h"


FREEZE_NET::SocketBase::SocketBase(IOThread *io_thread) :
Own(io_thread)
{
}

FREEZE_NET::SocketBase::~SocketBase ()
{
}

int FREEZE_NET::SocketBase::Bind(sockaddr_in addr)
{
	//  Choose I/O thread to run the listerner in.
	IOThread *io_thread = ChooseIOThread(0);
	if (!io_thread) 
	{
		return -1;
	}

	//  Create and run the listener.
	ZmqListener *listener = new (std::nothrow) ZmqListener(io_thread);
	assert(NULL != listener);
	int rc = listener->SetAddress(addr);
	if (rc != 0)
	{
		delete listener;
		return -1;
	}
	LaunchChild(listener);

	return 0;
}

int FREEZE_NET::SocketBase::Bind(u_short port, const char* ip, int backlog)
{
	//  Choose I/O thread to run the listerner in.
	IOThread *io_thread = ChooseIOThread(0);
	if (!io_thread) 
	{
		return -1;
	}

	//  Create and run the listener.
	ZmqListener *listener = new (std::nothrow) ZmqListener(io_thread);
	assert(NULL != listener);
	int rc = listener->SetAddress(port, ip);
	if (rc != 0)
	{
		delete listener;
		return -1;
	}
	LaunchChild(listener);

	return 0;
}

//int FREEZE_NET::SocketBase::Connect(sockaddr_in addr)
//{
//	//  Choose the I/O thread to run the session in.
//	IOThread *io_thread = ChooseIOThread(0);
//	if (!io_thread) 
//	{
//		return -1;
//	}
//
//	ZmqConnecter *connecter = new (std::nothrow) ZmqConnecter (io_thread, addr, false);
//	assert(NULL != connecter);
//	LaunchChild(connecter);
//
//	return 0;
//}

void FREEZE_NET::SocketBase::Stop()
{
	Own::Terminate();
}

int  FREEZE_NET::SocketBase::Connect(u_short port, const char* ip)
{
	struct sockaddr_in s;            
	memset(&s, 0, sizeof(struct sockaddr_in));  
	s.sin_family = AF_INET;     
	s.sin_port = htons(port);   
	s.sin_addr.s_addr = inet_addr(ip); 

	if (s.sin_addr.s_addr == INADDR_NONE)
	{               
		//warning gethostbyname is obsolete use new function getnameinfo            
		std::cout << "Asking ip to DNS for %s\n" << ip << std::endl;

		struct hostent *h = gethostbyname(ip);                   
		if (!h)
		{                       
			std::cout<<  "DNS resolution failed for %s\n" << ip << std::endl;

			return -1;
		}                 
		s.sin_addr.s_addr = *((int*)(*(h->h_addr_list)));          
	}

	//  Choose the I/O thread to run the session in.
	IOThread *io_thread = ChooseIOThread(0);
	if (!io_thread) 
	{
		return -1;
	}

	uint32_t serial_num = GetCtx()->GenSerialNum();

	GetCtx()->RegisterSendBuf(serial_num);

	ZmqConnecter *connecter = new (std::nothrow) ZmqConnecter (io_thread, s, false, serial_num);
	assert(NULL != connecter);
	LaunchChild(connecter);

	return serial_num;
}