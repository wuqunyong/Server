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

#include <assert.h>

#include "IOObject.h"
#include "IOThread.h"

FREEZE_NET::IOObject::IOObject(IOThread *io_thread) :
	poller_(NULL)
{
	if (io_thread)
	{
		Plug(io_thread);
	}

}

FREEZE_NET::IOObject::~IOObject()
{
}

void FREEZE_NET::IOObject::Plug(IOThread *io_thread)
{
	assert(io_thread != NULL);
	assert(poller_ == NULL);

	//  Retrieve the poller from the thread we are running in.
	poller_ = io_thread->GetPoller();
}

void FREEZE_NET::IOObject::Unplug()
{
	assert(poller_ != NULL);

	//  Forget about old poller in preparation to be migrated
	//  to a different I/O thread.
	poller_ = NULL;
}

FREEZE_NET::IOObject::handle_t FREEZE_NET::IOObject::AddFd(fd_t fd)
{
	return poller_->AddFd(fd, this);
}

void FREEZE_NET::IOObject::RmFd(handle_t handle)
{
	poller_->RmFd(handle);
}

void FREEZE_NET::IOObject::SetPollin(handle_t handle)
{
	poller_->SetPollin(handle);
}

void FREEZE_NET::IOObject::ResetPollin(handle_t handle)
{
	poller_->ResetPollin(handle);
}

void FREEZE_NET::IOObject::SetPollout(handle_t handle)
{
	poller_->SetPollout(handle);
}

void FREEZE_NET::IOObject::ResetPollout(handle_t handle)
{
	poller_->ResetPollout(handle);
}

void FREEZE_NET::IOObject::AddTimer(int timeout, int id)
{
	poller_->AddTimer(timeout, this, id);
}

void FREEZE_NET::IOObject::CancelTimer(int id)
{
	poller_->CancelTimer(this, id);
}

void FREEZE_NET::IOObject::InEvent()
{
	assert(false);
}

void FREEZE_NET::IOObject::OutEvent()
{
	assert(false);
}

void FREEZE_NET::IOObject::TimerEvent(int id)
{
	assert(false);
}
