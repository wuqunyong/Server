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

#ifndef __SHARED_NET_THREAD_H__
#define __SHARED_NET_THREAD_H__

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif


namespace FREEZE_NET
{

	typedef void ( thread_fn)(void*);

	//  Class encapsulating OS thread. Thread initiation/termination is done
	//  using special functions rather than in constructor/destructor so that
	//  thread isn't created during object construction by accident, causing
	//  newly created thread to access half-initialised object. Same applies
	//  to the destruction process: Thread should be terminated before object
	//  destruction begins, otherwise it can access half-destructed object.

	class Thread
	{
	public:

		inline Thread(void)
		{
		}

		//  Creates OS thread. 'tfn' is main thread function. It'll be passed
		//  'arg' as an argument.
		void Start(thread_fn *tfn, void *arg);

		//  Waits for thread termination.
		void Stop();

		//  These are internal members. They should be private, however then
		//  they would not be accessible from the main C routine of the thread.
		thread_fn *tfn_;
		void *arg_;

	private:

#ifdef WIN32
		HANDLE descriptor_;
#else
		pthread_t descriptor_;
#endif

		Thread(const Thread&);
		const Thread& operator=(const Thread&);
	};

}

#endif
