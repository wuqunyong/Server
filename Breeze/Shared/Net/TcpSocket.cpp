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

#include "TcpSocket.h"
#include <assert.h>

#ifdef WIN32

FREEZE_NET::TcpSocket::TcpSocket() :
    s_(retired_fd)
{
}

FREEZE_NET::TcpSocket::~TcpSocket()
{
    if (s_ != retired_fd)
	{
		 Close();
	}
}

int FREEZE_NET::TcpSocket::Open(fd_t fd, uint64_t sndbuf, uint64_t rcvbuf)
{
    assert(s_ == retired_fd);
    s_ = fd;

    if (sndbuf) 
	{
        int sz = (int) sndbuf;
        int rc = setsockopt(s_, SOL_SOCKET, SO_SNDBUF, (char*) &sz, sizeof(int));
        assert(rc == 0);
    }

    if (rcvbuf) 
	{
        int sz = (int) rcvbuf;
        int rc = setsockopt (s_, SOL_SOCKET, SO_RCVBUF, (char*) &sz, sizeof(int));
        assert(rc == 0);
    }

    return 0;
}

int FREEZE_NET::TcpSocket::Close()
{
    assert(s_ != retired_fd);
    int rc = closesocket(s_);
    assert(rc != SOCKET_ERROR);
    s_ = retired_fd;
    return 0;
}

FREEZE_NET::fd_t FREEZE_NET::TcpSocket::GetFd()
{
    return s_;
}

int FREEZE_NET::TcpSocket::Write(const void *data, int size)
{
    int nbytes = send(s_, (char*) data, size, 0);

    //  If not a single byte can be written to the socket in non-blocking mode
    //  we'll get an error (this may happen during the speculative write).
    if (nbytes == SOCKET_ERROR && WSAGetLastError () == WSAEWOULDBLOCK)
	{
		return 0;
	}
       
		
    //  Signalise peer failure.
    if (nbytes == -1 && (
          WSAGetLastError () == WSAENETDOWN ||
          WSAGetLastError () == WSAENETRESET ||
          WSAGetLastError () == WSAEHOSTUNREACH ||
          WSAGetLastError () == WSAECONNABORTED ||
          WSAGetLastError () == WSAETIMEDOUT ||
          WSAGetLastError () == WSAECONNRESET))
	{
		return -1;
	}
        

    assert(nbytes != SOCKET_ERROR);

    return (size_t) nbytes;
}

int FREEZE_NET::TcpSocket::Read(void *data, int size)
{
    int nbytes = recv(s_, (char*) data, size, 0);

    //  If not a single byte can be read from the socket in non-blocking mode
    //  we'll get an error (this may happen during the speculative read).
    if (nbytes == SOCKET_ERROR && WSAGetLastError () == WSAEWOULDBLOCK)
	{
		 return 0;
	}
       

    //  Connection failure.
    if (nbytes == -1 && (
          WSAGetLastError () == WSAENETDOWN ||
          WSAGetLastError () == WSAENETRESET ||
          WSAGetLastError () == WSAECONNABORTED ||
          WSAGetLastError () == WSAETIMEDOUT ||
          WSAGetLastError () == WSAECONNRESET ||
          WSAGetLastError () == WSAECONNREFUSED ||
          WSAGetLastError () == WSAENOTCONN))
	{
		return -1;
	}
        

    assert(nbytes != SOCKET_ERROR);

    //  Orderly shutdown by the other peer.
    if (nbytes == 0)
	{
		return -1; 
	}
       

    return (size_t) nbytes;
}

#else

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

FREEZE_NET::TcpSocket::TcpSocket() :
    s_(retired_fd)
{
}

FREEZE_NET::TcpSocket::~TcpSocket()
{
    if (s_ != retired_fd)
	{
		Close();
	}
        
}

int FREEZE_NET::TcpSocket::Open(fd_t fd, uint64_t sndbuf, uint64_t rcvbuf)
{
    assert (s_ == retired_fd);
    s_ = fd;

    if (sndbuf) 
	{
        int sz = (int) sndbuf;
        int rc = setsockopt(s_, SOL_SOCKET, SO_SNDBUF, &sz, sizeof(int));
        assert(rc == 0);
    }

    if (rcvbuf) 
	{
        int sz = (int) rcvbuf;
        int rc = setsockopt(s_, SOL_SOCKET, SO_RCVBUF, &sz, sizeof(int));
        assert(rc == 0);
    }

    return 0;
}

int FREEZE_NET::TcpSocket::Close()
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

FREEZE_NET::fd_t FREEZE_NET::TcpSocket::GetFd()
{
    return s_;
}

int FREEZE_NET::TcpSocket::Write(const void *data, int size)
{
    ssize_t nbytes = send(s_, data, size, 0);

    //  Several errors are OK. When speculative write is being done we may not
    //  be able to write a single byte to the socket. Also, SIGSTOP issued
    //  by a debugging tool can result in EINTR error.
    if (nbytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
	{
		 return 0;
	}     

    //  Signalise peer failure.
    if (nbytes == -1 && (errno == ECONNRESET || errno == EPIPE))
	{
		return -1;
	}

    assert(nbytes != -1);
    return (size_t) nbytes;
}

int FREEZE_NET::TcpSocket::Read(void *data, int size)
{
    ssize_t nbytes = recv(s_, data, size, 0);

    //  Several errors are OK. When speculative read is being done we may not
    //  be able to read a single byte to the socket. Also, SIGSTOP issued
    //  by a debugging tool can result in EINTR error.
    if (nbytes == -1
    && (errno == EAGAIN
     || errno == EWOULDBLOCK
     || errno == EINTR))
	{
		return 0;
	}

    //  Signal peer failure.
    if (nbytes == -1
    && (errno == ECONNRESET
     || errno == ECONNREFUSED
     || errno == ETIMEDOUT
     || errno == EHOSTUNREACH
     || errno == ENOTCONN))
	{
		return -1;
	}
        

   assert(nbytes != -1);

    //  Orderly shutdown by the other peer.
    if (nbytes == 0)
	{
		return -1;
	}
   
    return (size_t) nbytes;
}

#endif

