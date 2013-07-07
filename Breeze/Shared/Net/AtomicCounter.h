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

#ifndef __SHARED_NET_ATOMIC_COUNTER_H__
#define __SHARED_NET_ATOMIC_COUNTER_H__

#include <stdint.h>

#include "Mutex.h"

namespace FREEZE_NET
{

	//  This class represents an integer that can be incremented/decremented
	//  in atomic fashion.
	class AtomicCounter
	{
	public:

		typedef uint32_t integer_t;

		inline AtomicCounter(integer_t value = 0) :
			value_(value)
		{
		}

		inline ~AtomicCounter(void)
		{
		}

		//  Set counter value (not thread-safe).
		inline void Set(integer_t value)
		{
			value_ = value;
		}

		//  Atomic addition. Returns the old value.
		inline integer_t Add(integer_t increment)
		{
			integer_t old_value;

			sync_.Lock();
			old_value = value_;
			value_ += increment;
			sync_.Unlock();

			return old_value;
		}

		//  Atomic subtraction. Returns false if the counter drops to zero.
		inline bool Sub(integer_t decrement)
		{
			sync_.Lock();
			value_ -= decrement;
			bool result = value_ ? true : false;
			sync_.Unlock();

			return result;
		}

		inline integer_t Get()
		{
			return value_;
		}

	private:

		volatile integer_t value_;
		Mutex sync_;

		AtomicCounter(const AtomicCounter&);
		const AtomicCounter& operator=(const AtomicCounter&);
	};

}

#endif

