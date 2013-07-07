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

#ifndef __SHARED_NET_ZMQ_LISTENER_H__
#define __SHARED_NET_ZMQ_LISTENER_H__

#include "Own.h"
#include "IOObject.h"
#include "TcpListener.h"

namespace FREEZE_NET
{

    class ZmqListener : public Own, public IOObject
    {
    public:

        ZmqListener (class IOThread *io_thread);
        ~ZmqListener ();

        //  Set address to listen on.
        int SetAddress(sockaddr_in addr);
		//"127.0.0.1"
		int SetAddress(u_short port, const char* ip, int backlog = 128);

    private:

        //  Handlers for incoming commands.
        void ProcessPlug();
        void ProcessTerm(int linger);

        //  Handlers for I/O events.
        void InEvent();

        //  Actual listening socket.
        TcpListener tcp_listener_;

        //  Handle corresponding to the listening socket.
        handle_t handle_;

        ZmqListener (const ZmqListener&);
        const ZmqListener &operator = (const ZmqListener&);
    };

}

#endif
