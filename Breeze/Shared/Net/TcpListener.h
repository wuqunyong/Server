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

#ifndef __SHARED_NET_TCP_LISTEN_H__
#define __SHARED_NET_TCP_LISTEN_H__

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <assert.h>
#else
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include "Fd.h"

namespace FREEZE_NET
{

	//  The class encapsulating simple TCP listening socket.

	class TcpListener
	{
	public:

		TcpListener();
		~TcpListener();

		//  Start listening on the interface.
		int SetAddress(sockaddr_in addr, int backlog);

		//  Close the listening socket.
		int Close();

		//  Get the file descriptor to poll on to get notified about
		//  newly created connections.
		fd_t GetFd();

		//  Accept the new connection. Returns the file descriptor of the
		//  newly created connection. The function may return retired_fd
		//  if the connection was dropped while waiting in the listen backlog.
		fd_t Accept();

	private:

		//  Address to listen on.
		sockaddr_in addr_;
		int addr_len_;

		//  Underlying socket.
		fd_t s_;

		TcpListener(const TcpListener&);
		const TcpListener &operator =(const TcpListener&);
	};

}

#endif
