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

#ifndef __SHARED_NET_SIGNALER_H__
#define __SHARED_NET_SIGNALER_H__

#include "Fd.h"

namespace FREEZE_NET
{

    //  This is a cross-platform equivalent to signal_fd. However, as opposed
    //  to signal_fd there can be at most one signal in the signaler at any
    //  given moment. Attempt to send a signal before receiving the previous
    //  one will result in undefined behaviour.

    class Signaler
    {
    public:

    	Signaler();
        ~Signaler();

        fd_t GetFd();
        void Send();
        void Recv();

    private:

        //  Creates a pair of filedescriptors that will be used
        //  to pass the signals.
        static int make_fdpair(fd_t *r, fd_t *w);

        //  Write & read end of the socketpair.
        fd_t w_;
        fd_t r_;

        //  Disable copying of signaler_t object.
        Signaler (const Signaler&);
        const Signaler &operator = (const Signaler&);
    };

}

#endif
