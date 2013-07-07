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

#include "Fd.h"
#include "ZmqListener.h"
//#include "ZmqInit.h"
#include "IOThread.h"
#include "ZmqEngine.h"
#include "Ctx.h"

FREEZE_NET::ZmqListener::ZmqListener (IOThread *io_thread) :
    Own(io_thread),
    IOObject(io_thread)
{
}

FREEZE_NET::ZmqListener::~ZmqListener ()
{
}

int FREEZE_NET::ZmqListener::SetAddress(sockaddr_in addr)
{
     return tcp_listener_.SetAddress(addr, 256);
}

int FREEZE_NET::ZmqListener::SetAddress(u_short port, const char* ip, int backlog)
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

	return tcp_listener_.SetAddress(s, backlog);
}

void FREEZE_NET::ZmqListener::ProcessPlug()
{
    //  Start polling for incoming connections.
    handle_ = AddFd(tcp_listener_.GetFd());
    SetPollin(handle_);
}

void FREEZE_NET::ZmqListener::ProcessTerm(int linger)
{
    RmFd(handle_);
    Own::ProcessTerm(linger);
}

void FREEZE_NET::ZmqListener::InEvent()
{
    fd_t fd = tcp_listener_.Accept();

    //  If connection was reset by the peer in the meantime, just ignore it.
    //  TODO: Handle specific errors like ENFILE/EMFILE etc.
    if (fd == retired_fd)
	{
		 return;
	}

	std::cout << "accept fd : " << fd << std::endl;

    //  Choose I/O thread to run connecter in. Given that we are already
    //  running in an I/O thread, there must be at least one available.
    IOThread *io_thread = ChooseIOThread(0);
    assert(NULL != io_thread);

	uint32_t serial_num = GetCtx()->GenSerialNum();
    //  Create and launch an init object. 
    ZmqEngine *init = new (std::nothrow) ZmqEngine (io_thread, fd, serial_num);
    assert(NULL != init);
    LaunchSibling(init);
}

