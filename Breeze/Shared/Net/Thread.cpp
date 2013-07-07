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

#include "Thread.h"

#include <assert.h>

#ifdef WIN32

extern "C"
{
	static unsigned int __stdcall thread_routine(void *arg)
	{
		FREEZE_NET::Thread *self = (FREEZE_NET::Thread*)arg;
		self->tfn_(self->arg_);
		return 0;
	}
}

void FREEZE_NET::Thread::Start(thread_fn *tfn, void *arg)
{
	tfn_ = tfn;
	arg_ = arg;
	descriptor_ = (HANDLE)_beginthreadex(NULL, 0, &::thread_routine, this, 0 , NULL);
	assert(descriptor_ != NULL);
}

void FREEZE_NET::Thread::Stop()
{
	DWORD rc = WaitForSingleObject(descriptor_, INFINITE);
	assert(rc != WAIT_FAILED);
	BOOL rc2 = CloseHandle(descriptor_);
	assert(rc2 != 0);
}

#else
#include <signal.h>

extern "C"
{
	static void *thread_routine(void *arg)
	{
		FREEZE_NET::Thread *self = (FREEZE_NET::Thread*)arg;
		self->tfn_(self->arg_);
		return NULL;
	}
}

void FREEZE_NET::Thread::Start(thread_fn *tfn, void *arg)
{
	tfn_ = tfn;
	arg_ = arg;
	int rc = pthread_create(&descriptor_, NULL, thread_routine, this);
	assert(rc == 0);
}

void FREEZE_NET::Thread::Stop(void)
{
	int rc = pthread_join(descriptor_, NULL);
	assert(rc == 0);
}
#endif
