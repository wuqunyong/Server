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

#include "PollerBase.h"

#include <assert.h>

FREEZE_NET::PollerBase::PollerBase(void)
{
}

FREEZE_NET::PollerBase::~PollerBase(void)
{
	//  Make sure there is no more load on the shutdown.
	assert(GetLoad() == 0);
}

int FREEZE_NET::PollerBase::GetLoad(void)
{
	return load_.Get();
}

void FREEZE_NET::PollerBase::AdjustLoad(int amount)
{
	if (amount > 0)
	{
		load_.Add(amount);
	}
	else if (amount < 0)
	{
		load_.Sub(-amount);
	}
}

void FREEZE_NET::PollerBase::AddTimer(int timeout, IPollEvents* sink, int id)
{
	uint64_t expiration = clock_.NowMS() + timeout;
	timer_info_t info = { sink, id };
	timers_.insert(timers_t::value_type(expiration, info));
}

void FREEZE_NET::PollerBase::CancelTimer(IPollEvents* sink, int id)
{
	//  Complexity of this operation is O(n). We assume it is rarely used.
	for (timers_t::iterator it = timers_.begin(); it != timers_.end(); ++it)
	{
		if (it->second.sink_ == sink && it->second.id_ == id)
		{
			timers_.erase(it);
			return;
		}
	}
}

uint64_t FREEZE_NET::PollerBase::ExecuteTimers()
{
	//  Fast track.
	if (timers_.empty())
	{
		return 0;
	}

	//  Get the current time.
	uint64_t current = clock_.NowMS();

	//   Execute the timers that are already due.
	timers_t::iterator it = timers_.begin();
	while (it != timers_.end())
	{

		//  If we have to wait to execute the item, same will be true about
		//  all the following items (multimap is sorted). Thus we can stop
		//  checking the subsequent timers and return the time to wait for
		//  the next timer (at least 1ms).
		if (it->first > current)
		{
			return it->first - current;
		}

		//  Trigger the timer.
		it->second.sink_->TimerEvent(it->second.id_);

		//  Remove it from the list of active timers.
		timers_t::iterator o = it;
		++it;
		timers_.erase(o);
	}

	//  There are no more timers.
	return 0;
}
