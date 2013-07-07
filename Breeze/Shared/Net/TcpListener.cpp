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

#include "TcpListener.h"

#include <assert.h>
#include <string>
#include <iostream>

#ifdef WIN32

FREEZE_NET::TcpListener::TcpListener() :
	s_(retired_fd)
{
	memset(&addr_, 0, sizeof(addr_));
	addr_len_ = 0;
}

FREEZE_NET::TcpListener::~TcpListener()
{
	if (s_ != retired_fd)
	{
		Close();
	}
		
}

int FREEZE_NET::TcpListener::SetAddress(sockaddr_in addr, int backlog)
{
	this->addr_ = addr;
	this->addr_len_ = sizeof(this->addr_);

	//  Create a listening socket.
	s_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s_ == INVALID_SOCKET) 
	{
		assert(0);
		return -1;
	}

	//  Allow reusing of the address.
	int flag = 1;
	int rc = setsockopt(s_, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*) &flag, sizeof (int));
	assert(rc != SOCKET_ERROR);

	//  Set the non-blocking flag.
	u_long uflag = 1;
	rc = ioctlsocket(s_, FIONBIO, &uflag);
	assert(rc != SOCKET_ERROR);

	//  Bind the socket to the network interface and port.
	rc = bind (s_, (struct sockaddr*) &addr_, addr_len_);
	if (rc == SOCKET_ERROR) 
	{
		assert(0);
		return -1;
	}

	//  Listen for incomming connections.
	rc = listen(s_, backlog);
	if (rc == SOCKET_ERROR) 
	{
		assert(0);
		return -1;
	}

	return 0;
}

int FREEZE_NET::TcpListener::Close()
{
	assert(s_ != retired_fd);
	int rc = closesocket(s_);
	assert(rc != SOCKET_ERROR);
	s_ = retired_fd;
	return 0;
}

FREEZE_NET::fd_t FREEZE_NET::TcpListener::GetFd()
{
	return s_;
}

FREEZE_NET::fd_t FREEZE_NET::TcpListener::Accept()
{
	assert(s_ != retired_fd);

	//  Accept one incoming connection.
	fd_t sock = ::accept(s_, NULL, NULL);
	if (sock == INVALID_SOCKET && 
		(WSAGetLastError () == WSAEWOULDBLOCK ||
		WSAGetLastError () == WSAECONNRESET))
		return retired_fd;

	assert(sock != INVALID_SOCKET);

	// Set to non-blocking mode.
	unsigned long argp = 1;
	int rc = ioctlsocket (sock, FIONBIO, &argp);
	assert(rc != SOCKET_ERROR);

	//  Disable Nagle's algorithm.
	int flag = 1;
	rc = setsockopt (sock, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof (int));
	assert(rc != SOCKET_ERROR);

	return sock;
}

#else

#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <error.h>
#include <stdio.h>
#include <string.h>

#include "Fd.h"

FREEZE_NET::TcpListener::TcpListener() :
	s_(retired_fd)
{
	memset(&addr_, 0, sizeof(addr_));
}

FREEZE_NET::TcpListener::~TcpListener()
{
	if (s_ != retired_fd)
	{
		Close();
	}

}

int FREEZE_NET::TcpListener::SetAddress(sockaddr_in addr, int backlog)
{
	this->addr_ = addr;
	this->addr_len_ = sizeof(this->addr_);

	//  Create a listening socket.
	s_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s_ == -1)
	{
		return -1;
	}

	//  Allow reusing of the address.
	int flag = 1;
	int rc = setsockopt(s_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
	assert(rc == 0);

	//  Set the non-blocking flag.
	flag = fcntl(s_, F_GETFL, 0);
	if (flag == -1)
	{
		flag = 0;
	}

	rc = fcntl(s_, F_SETFL, flag | O_NONBLOCK);
	assert(rc != -1);

	//  Bind the socket to the network interface and port.
	rc = bind(s_, (struct sockaddr*) &addr_, addr_len_);
	if (rc != 0)
	{
		int err = errno;
		if (Close() != 0)
		{
			return -1;
		}

		errno = err;
		return -1;
	}

	//  Listen for incomming connections.
	rc = listen(s_, backlog);
	if (rc != 0)
	{
		int err = errno;
		if (Close() != 0)
		{
			return -1;
		}

		errno = err;
		return -1;
	}

	return 0;
}

int FREEZE_NET::TcpListener::Close()
{
	assert(s_ != retired_fd);
	int rc = ::close(s_);
	if (rc != 0)
	{
		return -1;
	}

	s_ = retired_fd;

	return 0;
}

FREEZE_NET::fd_t FREEZE_NET::TcpListener::GetFd()
{
	return s_;
}

FREEZE_NET::fd_t FREEZE_NET::TcpListener::Accept()
{
	assert(s_ != retired_fd);

	//  Accept one incoming connection.
	fd_t sock = ::accept(s_, NULL, NULL);

	if (sock == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno
			== EINTR || errno == ECONNABORTED))
	{
		return retired_fd;
	}

	//assert(sock != -1);
	if (sock == -1)
	{
		std::cout << "accept errno: " << errno << std::endl;
		perror(strerror(errno));
		return retired_fd;
	}
	

	// Set to non-blocking mode.
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1)
	{
		flags = 0;
	}

	int rc = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
	assert(rc != -1);

	//  Disable Nagle's algorithm.
	int flag = 1;
	rc = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int));
	assert(rc == 0);

	return sock;
}
#endif