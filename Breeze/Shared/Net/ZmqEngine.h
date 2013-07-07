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

#ifndef __SHARED_NET_ZMQENGINE_H__
#define __SHARED_NET_ZMQENGINE_H__

#include <stddef.h>

#include <string>

#include "IEngine.h"
#include "IOObject.h"
#include "TcpSocket.h"
#include "Object.h"
#include "Own.h"

#include "../Serialization/ByteBuffer.h"

namespace FREEZE_NET
{

    class ZmqEngine : public IOObject, public IEngine, public Own
    {
    public:

        ZmqEngine(class IOThread *io_thread, fd_t fd, uint32_t serial_num);
        ~ZmqEngine();

        //  i_engine interface implementation.
        void Plug(class IOThread *io_thread);
        void Unplug();
        void Terminate();
        void ActivateIn();
        void ActivateOut();

        //  i_poll_events interface implementation.
        void InEvent();
        void OutEvent();

    private:

		const uint32_t serial_num_;

		//  Handlers for incoming commands.
		void ProcessPlug();
		void ProcessActivateOut();
		void ProcessTerm(int linger);

        //  Function to handle network disconnections.
        void Error();

		uint32_t FindRoutingNum(uint32_t serial_num);

        TcpSocket tcp_socket_;
        handle_t handle_;

        //unsigned char *inpos_;
        //size_t insize_;
        ByteBuffer decoder_;

        //unsigned char *outpos;
        //size_t outsize;
        ByteBuffer encoder_;

        bool plugged_;

		class IOThread *io_thread_;

		//key : 原包的当前通信SESSION
		//value: 映射到当前通信的SESSION
		std::map<uint32_t, uint32_t> routing_serial_num_;

        ZmqEngine (const ZmqEngine&);
        const ZmqEngine &operator = (const ZmqEngine&);
    };

}

#endif
