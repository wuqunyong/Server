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

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#endif


#include "Signaler.h"

FREEZE_NET::Signaler::Signaler()
{
    //  Create the socketpair for signaling.
    int rc = make_fdpair(&r_, &w_);
    assert(rc == 0);

	//  Set both fds to non-blocking mode.
#if defined WIN32
	unsigned long argp = 1;
	rc = ioctlsocket (w_, FIONBIO, &argp);
	assert(rc != SOCKET_ERROR);
	rc = ioctlsocket (r_, FIONBIO, &argp);
	assert(rc != SOCKET_ERROR);
#else
    int flags = fcntl(w_, F_GETFL, 0);
    assert(flags >= 0);
    rc = fcntl(w_, F_SETFL, flags | O_NONBLOCK);
    assert(rc == 0);
    flags = fcntl(r_, F_GETFL, 0);
    assert(flags >= 0);
    rc = fcntl(r_, F_SETFL, flags | O_NONBLOCK);
    assert(rc == 0);
#endif
}

FREEZE_NET::Signaler::~Signaler()
{
#if defined WIN32
	int rc = closesocket(w_);
	assert(rc != SOCKET_ERROR);
	rc = closesocket(r_);
	assert(rc != SOCKET_ERROR);
#else
    close(w_);
    close(r_);
#endif
}

FREEZE_NET::fd_t FREEZE_NET::Signaler::GetFd()
{
    return r_;
}

void FREEZE_NET::Signaler::Send()
{
#if defined WIN32
	unsigned char dummy = 0;
	int nbytes = ::send (w_, (char*) &dummy, sizeof (dummy), 0);
	assert(nbytes != SOCKET_ERROR);
	assert(nbytes == sizeof(dummy));
#else
    unsigned char dummy = 0;
    while (true)
    {
        ssize_t nbytes = ::send(w_, &dummy, sizeof(dummy), 0);
        if (nbytes == -1 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
        {
        	 continue;
        }
        assert(nbytes == sizeof(dummy));
        break;
    }
#endif
}

void FREEZE_NET::Signaler::Recv()
{
    //  Attempt to read a signal.
    unsigned char dummy;
#ifdef WIN32
	int nbytes = ::recv (r_, (char*) &dummy, sizeof (dummy), 0);
	//assert(nbytes != SOCKET_ERROR);
#else
    ssize_t nbytes = ::recv(r_, &dummy, sizeof(dummy), 0);
    //if (nbytes == -1 && (errno == EAGAIN || errno == EINTR))
    //{
    //	return;
    //}
	//assert(nbytes >= 0);
#endif

    //assert(nbytes == sizeof(dummy));
    //assert(dummy == 0);
}

int FREEZE_NET::Signaler::make_fdpair(fd_t *r, fd_t *w)
{
#if defined WIN32

	//  Windows has no 'socketpair' function. CreatePipe is no good as pipe
	//  handles cannot be polled on. Here we create the socketpair by hand.
	*w = INVALID_SOCKET;
	*r = INVALID_SOCKET;

	//  Create listening socket.
	SOCKET listener;
	listener = socket(AF_INET, SOCK_STREAM, 0);
	assert(listener != INVALID_SOCKET);

	//  Set SO_REUSEADDR and TCP_NODELAY on listening socket.
	BOOL so_reuseaddr = 1;
	int rc = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
		(char *)&so_reuseaddr, sizeof (so_reuseaddr));
	assert(rc != SOCKET_ERROR);
	BOOL tcp_nodelay = 1;
	rc = setsockopt(listener, IPPROTO_TCP, TCP_NODELAY,
		(char *)&tcp_nodelay, sizeof (tcp_nodelay));
	assert(rc != SOCKET_ERROR);

	//  Bind listening socket to any free local port.
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof (addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0;
	rc = bind(listener, (const struct sockaddr*) &addr, sizeof (addr));
	assert(rc != SOCKET_ERROR);

	//  Retrieve local port listener is bound to (into addr).
	int addrlen = sizeof(addr);
	rc = getsockname(listener, (struct sockaddr*) &addr, &addrlen);
	assert(rc != SOCKET_ERROR);

	//  Listen for incomming connections.
	rc = listen(listener, 1);
	assert(rc != SOCKET_ERROR);

	//  Create the writer socket.
	*w = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,  0);
	assert(*w != INVALID_SOCKET);

	//  Set TCP_NODELAY on writer socket.
	rc = setsockopt(*w, IPPROTO_TCP, TCP_NODELAY,
		(char *)&tcp_nodelay, sizeof (tcp_nodelay));
	assert(rc != SOCKET_ERROR);

	//  Connect writer to the listener.
	rc = connect(*w, (sockaddr *) &addr, sizeof (addr));
	assert(rc != SOCKET_ERROR);

	//  Accept connection from writer.
	*r = accept(listener, NULL, NULL);
	assert(*r != INVALID_SOCKET);

	//  We don't need the listening socket anymore. Close it.
	rc = closesocket(listener);
	assert(rc != SOCKET_ERROR);

	return 0;
#else
    int sv [2];
    int rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    assert(rc == 0);
    *w = sv [0];
    *r = sv [1];
    return 0;
#endif
}


