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

#ifndef __SHARED_NET_ZMQ_CONNECTER_H__
#define __SHARED_NET_ZMQ_CONNECTER_H__

#include "Own.h"
#include "IOObject.h"
#include "TcpConnecter.h"

namespace FREEZE_NET
{

    class ZmqConnecter : public Own, public IOObject
    {
    public:

        //  If 'wait' is true connecter first waits for a while, then starts
        //  connection process.
        ZmqConnecter(class IOThread *io_thread, sockaddr_in addr, bool delay, uint32_t serial_num);
        ~ZmqConnecter();

    private:

		const uint32_t serial_num_;
        //  ID of the timer used to delay the reconnection.
        enum {reconnect_timer_id = 1};

        //  Handlers for incoming commands.
        void ProcessPlug();

        //  Handlers for I/O events.
        void InEvent();
        void OutEvent();
        void TimerEvent(int id);

        //  Internal function to start the actual connection establishment.
        void StartConnecting();

        //  Internal function to add a reconnect timer
        void AddReconnectTimer();

        //  Internal function to return a reconnect backoff delay.
        //  Will modify the current_reconnect_ivl used for next call
        //  Returns the currently used interval
        int GetNewReconnectIvl();

        //  Actual connecting socket.
        TcpConnecter tcp_connecter_;

        //  Handle corresponding to the listening socket.
        handle_t handle_;

        //  If true file descriptor is registered with the poller and 'handle'
        //  contains valid value.
        bool handle_valid_;

        //  If true, connecter is waiting a while before trying to connect.
        bool wait_;

        //  Current reconnect ivl, updated for backoff strategy
        int current_reconnect_ivl_;

        ZmqConnecter (const ZmqConnecter&);
        const ZmqConnecter &operator = (const ZmqConnecter&);
    };

}

#endif
