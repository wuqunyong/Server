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

#ifndef __SHARED_NET_MUTEX_H__
#define __SHARED_NET_MUTEX_H__

#ifdef WIN32

#include <windows.h>

namespace FREEZE_NET
{

	class Mutex
	{
	public:
		inline Mutex()
		{
			InitializeCriticalSection (&cs);
		}

		inline ~Mutex()
		{
			DeleteCriticalSection (&cs);
		}

		inline void Lock()
		{
			EnterCriticalSection (&cs);
		}

		inline void Unlock()
		{
			LeaveCriticalSection (&cs);
		}

	private:

		CRITICAL_SECTION cs;

		//  Disable copy construction and assignment.
		Mutex (const Mutex&);
		void operator = (const Mutex&);
	};

}

#else

#include <assert.h>
#include <pthread.h>

namespace FREEZE_NET
{
	class Mutex
	{
	public:
		inline Mutex()
		{
			int rc = pthread_mutex_init(&mutex_, NULL);
			if (rc)
			{
				assert(0 == rc);
			}

		}

		inline ~Mutex()
		{
			int rc = pthread_mutex_destroy(&mutex_);
			if (rc)
			{
				assert(0 == rc);
			}
		}

		inline void Lock()
		{
			int rc = pthread_mutex_lock(&mutex_);
			if (rc)
			{
				assert(0 == rc);
			}
		}

		inline void Unlock()
		{
			int rc = pthread_mutex_unlock(&mutex_);
			if (rc)
			{
				assert(0 == rc);
			}
		}

	private:

		pthread_mutex_t mutex_;

		// Disable copy construction and assignment.
		Mutex (const Mutex&);
		const Mutex &operator = (const Mutex&);
	};

}
#endif

#endif
