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

#ifndef __SHARED_NET_SELECT_H__
#define __SHARED_NET_SELECT_H__

#include <stddef.h>
#include <vector>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/select.h>
#include <errno.h>
#endif

#include "Fd.h"
#include "Thread.h"
#include "PollerBase.h"

namespace FREEZE_NET
{

    //  Implements socket polling mechanism using POSIX.1-2001 select()
    //  function.

    class Select : public PollerBase
    {
    public:

        typedef fd_t handle_t;

        Select();
        ~Select();

        //  "poller" concept.
		handle_t AddFd(fd_t fd, struct IPollEvents* events);
		void RmFd(handle_t handle);
		void SetPollin(handle_t handle);
		void ResetPollin(handle_t handle);
		void SetPollout(handle_t handle);
		void ResetPollout(handle_t handle);
		void Start();
		void Stop();
    private:

        //  Main worker thread routine.
        static void worker_routine(void *arg);

        //  Main event loop.
        void Loop();

        struct fd_entry_t
        {
            fd_t fd_;
            struct IPollEvents *events_;
        };

        //  Checks if an fd_entry_t is retired.
        static bool is_retired_fd(const fd_entry_t &entry);

        //  Set of file descriptors that are used to retreive
        //  information for fd_set.
        typedef std::vector <fd_entry_t> fd_set_t;
        fd_set_t fds_;

        fd_set source_set_in_;
        fd_set source_set_out_;
        fd_set source_set_err_;

        fd_set readfds_;
        fd_set writefds_;
        fd_set exceptfds_;

        //  Maximum file descriptor.
        fd_t maxfd_;

        //  If true, at least one file descriptor has retired.
        bool retired_;

        //  If true, thread is shutting down.
        bool stopping_;

        //  Handle of the physical thread doing the I/O work.
        Thread worker_;

        Select(const Select&);
        const Select &operator = (const Select&);
    };

}

#endif