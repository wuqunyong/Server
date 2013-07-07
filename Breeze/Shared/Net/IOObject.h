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

#ifndef __SHARED_NET_IO_OBJECT_H__
#define __SHARED_NET_IO_OBJECT_H__

#include <stddef.h>

#include <stdint.h>
#include "Poller.h"
#include "IPollEvents.h"

namespace FREEZE_NET {

	//  Simple base class for objects that live in I/O threads.
	//  It makes communication with the poller object easier and
	//  makes defining unneeded event handlers unnecessary.

	class IOObject: public IPollEvents 
	{
	public:

		IOObject(class IOThread *io_thread = NULL);
		virtual ~IOObject();

		//  When migrating an object from one I/O thread to another, first
		//  unplug it, then migrate it, then plug it to the new thread.
		void Plug(class IOThread *io_thread);
		void Unplug();

	protected:

		typedef Poller::handle_t handle_t;

		//  Methods to access underlying poller object.
		handle_t AddFd(fd_t fd);
		void RmFd(handle_t handle);
		void SetPollin(handle_t handle);
		void ResetPollin(handle_t handle);
		void SetPollout(handle_t handle);
		void ResetPollout(handle_t handle);
		void AddTimer(int timout, int id);
		void CancelTimer(int id);

		//  i_poll_events interface implementation.
		void InEvent();
		void OutEvent();
		void TimerEvent(int id);

	private:

		Poller *poller_;

		IOObject(const IOObject&);
		const IOObject &operator =(const IOObject&);
	};

}

#endif
