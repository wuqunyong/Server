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

#ifndef __SHARED_NET_IO_THREAD_H__
#define __SHARED_NET_IO_THREAD_H__

#include <vector>
#include <stdint.h>

#include "Object.h"
#include "Poller.h"
#include "IPollEvents.h"
#include "Mailbox.h"

namespace FREEZE_NET
{

	//  Generic part of the I/O thread. Polling-mechanism-specific features
	//  are implemented in separate "polling objects".

	class Ctx;
	class IOThread: public Object, public IPollEvents
	{
	public:

		IOThread(Ctx* ctx, uint32_t tid);

		//  Clean-up. If the thread was started, it's neccessary to call 'stop'
		//  before invoking destructor. Otherwise the destructor would hang up.
		virtual ~IOThread();

		//  Launch the physical thread.
		void Start();

		//  Ask underlying thread to stop.
		void Stop();

		//  Returns mailbox associated with this I/O thread.
		Mailbox* GetMailbox();

		//  i_poll_events implementation.
		void InEvent();
		void OutEvent();
		void TimerEvent(int id);

		//  Used by io_objects to retrieve the assciated poller object.
		Poller* GetPoller();

		//  Command handlers.
		void ProcessStop();

		//  Returns load experienced by the I/O thread.
		int GetLoad();

	private:

		//  I/O thread accesses incoming commands via this mailbox.
		Mailbox mailbox_;

		//  Handle associated with mailbox' file descriptor.
		Poller::handle_t mailbox_handle_;

		//  I/O multiplexing is performed using a poller object.
		Poller* poller_;

		IOThread(const IOThread&);
		const IOThread& operator=(const IOThread&);
	};

}

#endif
