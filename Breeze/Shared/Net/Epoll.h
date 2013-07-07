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

#ifndef __SHARED_NET_EPOLL_H__
#define __SHARED_NET_EPOLL_H__

#ifndef WIN32

#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <iostream>
#include <stdint.h>

#include "Fd.h"
#include "Thread.h"
#include "PollerBase.h"

namespace FREEZE_NET
{

	//  This class implements socket polling mechanism using the Linux-specific
	//  epoll mechanism.

	class Epoll: public PollerBase
	{
	public:

		typedef void* handle_t;

		Epoll();
		~Epoll();

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

		//  Main epoll file descriptor
		fd_t epoll_fd_;

		struct poll_entry_t
		{
			fd_t fd_;
			epoll_event ev_;
			struct IPollEvents* events_;
		};

		//  List of retired event sources.
		typedef std::vector<poll_entry_t*> retired_t;
		retired_t retired_;

		//  If true, thread is in the process of shutting down.
		bool stopping_;

		//  Handle of the physical thread doing the I/O work.
		Thread worker_;

		Epoll(const Epoll&);
		const Epoll& operator=(const Epoll&);
	};

}
#endif

#endif
