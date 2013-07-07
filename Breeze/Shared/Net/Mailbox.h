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

#ifndef __SHARED_NET_MAILBOX_H__
#define __SHARED_NET_MAILBOX_H__

#include <stddef.h>
#include <deque>

#include "Signaler.h"
#include "Fd.h"
#include "Command.h"
#include "Mutex.h"

namespace FREEZE_NET
{

    class Mailbox
    {
    public:

        Mailbox(void);
        ~Mailbox(void);

        fd_t GetFd(void);
        void Send(const Command& cmd);
        int Recv(Command* cmd);

    private:

        //  The pipe to store actual commands.
        std::deque<Command> cpipe_;

        //  Signaler to pass signals from writer thread to reader thread.
        Signaler signaler_;

        //  There's only one thread receiving from the mailbox, but there
        //  is arbitrary number of threads sending. Given that ypipe requires
        //  synchronised access on both of its endpoints, we have to synchronise
        //  the sending side.
        Mutex sync_;

        //  Disable copying of Mailbox object.
        Mailbox (const Mailbox&);
        const Mailbox& operator= (const Mailbox&);
    };

}

#endif
