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

#ifndef __SHARED_NET_POLLER_BASE_H__
#define __SHARED_NET_POLLER_BASE_H__

#include <map>

#include "AtomicCounter.h"
#include "Clock.h"
#include "IPollEvents.h"

namespace FREEZE_NET
{

	class PollerBase
	{
	public:

		PollerBase(void);
		virtual ~PollerBase(void);

		//  Returns load of the poller. Note that this function can be
		//  invoked from a different thread!
		int GetLoad(void);

		//  Add a timeout to expire in timeout_ milliseconds. After the
		//  expiration timer_event on sink_ object will be called with
		//  argument set to id_.
		void AddTimer(int timeout, struct IPollEvents* sink, int id);

		//  Cancel the timer created by sink_ object with ID equal to id_.
		void CancelTimer(struct IPollEvents* sink, int id);

	protected:

		//  Called by individual poller implementations to manage the load.
		void AdjustLoad(int amount);

		//  Executes any timers that are due. Returns number of milliseconds
		//  to wait to match the next timer or 0 meaning "no timers".
		uint64_t ExecuteTimers();

	private:

		//  Clock instance private to this I/O thread.
		Clock clock_;

		//  List of active timers.
		struct timer_info_t
		{
			struct IPollEvents* sink_;
			int id_;
		};
		typedef std::multimap<uint64_t, timer_info_t> timers_t;
		timers_t timers_;

		//  Load of the poller. Currently the number of file descriptors
		//  registered.
		AtomicCounter load_;

		PollerBase(const PollerBase&);
		const PollerBase &operator=(const PollerBase&);
	};

}

#endif
