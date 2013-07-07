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

#ifndef __SHARED_NET_TCPSOCKET_H__
#define __SHARED_NET_TCPSOCKET_H__

#include "Fd.h"


namespace FREEZE_NET
{

    //  The class encapsulating simple TCP read/write socket.

    class TcpSocket
    {
    public:

        TcpSocket();
        ~TcpSocket();

        //  Associates a socket with a native socket descriptor.
        int Open(fd_t fd, uint64_t sndbuf = 64*1024, uint64_t rcvbuf = 64*1024);
         
        //  Closes the underlying socket.
        int Close();

        //  Returns the underlying socket. Returns retired_fd when the socket
        //  is in the closed state.
        fd_t GetFd();

        //  Writes data to the socket. Returns the number of bytes actually
        //  written (even zero is to be considered to be a success). In case
        //  of error or orderly shutdown by the other peer -1 is returned.
        int Write(const void *data, int size);

        //  Reads data from the socket (up to 'size' bytes). Returns the number
        //  of bytes actually read (even zero is to be considered to be
        //  a success). In case of error or orderly shutdown by the other
        //  peer -1 is returned.
        int Read(void *data, int size);

    private:

        //  Underlying socket.
        fd_t s_;

        //  Disable copy construction of tcp_socket.
        TcpSocket (const TcpSocket&);
        const TcpSocket &operator = (const TcpSocket&);
    };

}

#endif
