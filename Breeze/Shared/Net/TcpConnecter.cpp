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

#include <string.h>
#include <string>
#include <assert.h>
#include <iostream>

#include "TcpConnecter.h"


#ifdef WIN32

#include <ws2tcpip.h>

FREEZE_NET::TcpConnecter::TcpConnecter() :
    s_ (retired_fd)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_len_ = 0;
}

FREEZE_NET::TcpConnecter::~TcpConnecter ()
{
    if (s_ != retired_fd)
	{
		Close();
	}
}

int FREEZE_NET::TcpConnecter::SetAddress(sockaddr_in addr)
{
	this->addr_ = addr;
	this->addr_len_ = sizeof(this->addr_);
    return 0;
}

int FREEZE_NET::TcpConnecter::Open()
{
    assert(s_ == retired_fd);

    //  Create the socket.
    s_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s_ == INVALID_SOCKET) 
	{
        assert(0);
        return -1;
    }

    // Set to non-blocking mode.
    unsigned long argp = 1;
    int rc = ioctlsocket(s_, FIONBIO, &argp);
    assert(rc != SOCKET_ERROR);

    //  Disable Nagle's algorithm.
    int flag = 1;
    rc = setsockopt(s_, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int));
   assert(rc != SOCKET_ERROR);

    //  Connect to the remote peer.
    rc = ::connect(s_, (sockaddr*) &addr_, addr_len_);

    //  Connect was successfull immediately.
    if (rc == 0)
	{
		return 0;
	}

    //  Asynchronous connect was launched.
    if (rc == SOCKET_ERROR && (WSAGetLastError () == WSAEINPROGRESS || WSAGetLastError () == WSAEWOULDBLOCK)) 
	{
        errno = EAGAIN;
        return -1;
    }

	std::cout << "TcpConnecter connect error!" << __FILE__ << " " << __LINE__ << std::endl; 
    //assert(0);
    return -1;
}

int FREEZE_NET::TcpConnecter::Close()
{
    assert(s_ != retired_fd);
    int rc = closesocket(s_);
    assert(rc != SOCKET_ERROR);
    s_ = retired_fd;
    return 0;
}

FREEZE_NET::fd_t FREEZE_NET::TcpConnecter::GetFd()
{
    return s_;
}

FREEZE_NET::fd_t FREEZE_NET::TcpConnecter::Connect()
{
    //  Nonblocking connect have finished. Check whether an error occured.
    int err = 0;
    socklen_t len = sizeof err;
    int rc = getsockopt(s_, SOL_SOCKET, SO_ERROR, (char*) &err, &len);
    assert(rc == 0);
    if (err != 0) 
	{

        //  Assert that the error was caused by the networking problems
        //  rather than 0MQ bug.
        if (err == WSAECONNREFUSED || err == WSAETIMEDOUT ||
              err == WSAECONNABORTED || err == WSAEHOSTUNREACH ||
              err == WSAENETUNREACH || err == WSAENETDOWN)
		{
			 return retired_fd;
		}

        assert(0);
    }

    //  Return the newly connected socket.
    fd_t result = s_;
    s_ = retired_fd;
    return result;
}

#else

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <error.h>
#include <stdio.h>
#include <string.h>

FREEZE_NET::TcpConnecter::TcpConnecter() :
    s_(retired_fd)
{
    memset(&addr_, 0, sizeof(addr_));
	addr_len_ = 0;
}

FREEZE_NET::TcpConnecter::~TcpConnecter()
{
    if (s_ != retired_fd)
	{
		Close();
	}
}

int FREEZE_NET::TcpConnecter::SetAddress(sockaddr_in addr) 
{
	this->addr_ = addr;
	this->addr_len_ = sizeof(this->addr_);
    return 0;
}

int FREEZE_NET::TcpConnecter::Open()
{
	assert(s_ == retired_fd);

	//  Create the socket.
	s_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s_ == -1)
	{
		std::cout << "socket errno: " << errno << std::endl;
		perror(strerror(errno));

		return -1;
	}
		

	int flags = fcntl(s_, F_GETFL, 0);
	if (flags == -1)
	{
		flags = 0;
	}
	int rc = fcntl(s_, F_SETFL, flags | O_NONBLOCK);
	assert(rc != -1);

	//  Disable Nagle's algorithm.
	int flag = 1;
	rc = setsockopt(s_, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int));
	assert(rc == 0);

	//  Connect to the remote peer.
	rc = ::connect(s_, (struct sockaddr*) &addr_, addr_len_);

	//  Connect was successfull immediately.
	if (rc == 0)
	{
		return 0;
	}
		

	//  Asynchronous connect was launched.
	if (rc == -1 && errno == EINPROGRESS) 
	{
		errno = EAGAIN;
		return -1;
	}

	//  Error occured.
	int err = errno;
	Close();
	errno = err;
	return -1;
}

int FREEZE_NET::TcpConnecter::Close()
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

FREEZE_NET::fd_t FREEZE_NET::TcpConnecter::GetFd()
{
    return s_;
}

FREEZE_NET::fd_t FREEZE_NET::TcpConnecter::Connect()
{
    //  Following code should handle both Berkeley-derived socket
    //  implementations and Solaris.
    int err = 0;
    socklen_t len = sizeof(err);
    int rc = getsockopt(s_, SOL_SOCKET, SO_ERROR, (char*) &err, &len);
    if (rc == -1)
	{
		err = errno;
	}
        
    if (err != 0) 
	{

        //  Assert if the error was caused by 0MQ bug.
        //  Networking problems are OK. No need to assert.
        errno = err;
        assert (errno == ECONNREFUSED || errno == ECONNRESET ||
            errno == ETIMEDOUT || errno == EHOSTUNREACH ||
            errno == ENETUNREACH);
        return retired_fd;
    }

    fd_t result = s_;
    s_ = retired_fd;
    return result;
}

#endif
