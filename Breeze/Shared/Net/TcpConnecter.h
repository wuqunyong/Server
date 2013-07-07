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

#ifndef __SHARED_NET_TCP_CONNECTER_H__
#define __SHARED_NET_TCP_CONNECTER_H__

#include "Fd.h"

#ifdef WIN32
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

namespace FREEZE_NET
{

    //  The class encapsulating simple TCP listening socket.

    class TcpConnecter
    {
    public:

        TcpConnecter();
        ~TcpConnecter();

        //  Set address to connect to.
        int SetAddress(sockaddr_in addr);

        //  Open TCP connecting socket. Address is in
        //  <hostname>:<port-number> format. Returns -1 in case of error,
        //  0 if connect was successfull immediately and 1 if async connect
        //  was launched.
        int Open();

        //  Close the connecting socket.
        int Close();

        //  Get the file descriptor to poll on to get notified about
        //  connection success.
        fd_t GetFd();

        //  Get the file descriptor of newly created connection. Returns
        //  retired_fd if the connection was unsuccessfull.
        fd_t Connect();

    private:

        //  Address to connect to.
        sockaddr_in addr_;
        int addr_len_;

        //  Underlying socket.
        fd_t s_;

        TcpConnecter (const TcpConnecter&);
        const TcpConnecter &operator = (const TcpConnecter&);
    };

}

#endif
