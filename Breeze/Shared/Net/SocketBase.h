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

#ifndef __SHARED_NET_SOCKET_BASE_H__
#define __SHARED_NET_SOCKET_BASE_H__

#include <map>
#include <vector>


#include "Own.h"
#include "Mutex.h"
#include "Poller.h"
#include "AtomicCounter.h"
#include "IPollEvents.h"
#include "Mailbox.h"

namespace FREEZE_NET
{

    class SocketBase :
        public Own
    {

    public:
		SocketBase (class IOThread *io_thread);
		virtual ~SocketBase();

        int Bind(sockaddr_in addr);
		int Bind(u_short port, const char* ip, int backlog = 128);
        //int Connect(sockaddr_in addr);
		int Connect(u_short port, const char* ip);

		void Stop();

    private:

        SocketBase (const SocketBase&);
        const SocketBase &operator = (const SocketBase&);
    };

}

#endif
