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
#include <new>

#include "Fd.h"
#include "IOThread.h"
#include "Ctx.h"
#include "ZmqEngine.h"


FREEZE_NET::IOThread::IOThread(Ctx* ctx, uint32_t tid) :
	Object(ctx, tid)
{
	poller_ = new (std::nothrow) Poller;
	assert(poller_ != NULL);

	mailbox_handle_ = poller_->AddFd(mailbox_.GetFd(), this);
	poller_->SetPollin(mailbox_handle_);
}

FREEZE_NET::IOThread::~IOThread()
{
	delete poller_;
}

void FREEZE_NET::IOThread::Start()
{
	//  Start the underlying I/O thread.
	poller_->Start();
}

void FREEZE_NET::IOThread::Stop()
{
	SendStop();
}

FREEZE_NET::Mailbox *FREEZE_NET::IOThread::GetMailbox()
{
	return &mailbox_;
}

int FREEZE_NET::IOThread::GetLoad()
{
	return poller_->GetLoad();
}

void FREEZE_NET::IOThread::InEvent()
{
	//  TODO: Do we want to limit number of commands I/O thread can
	//  process in a single go?

	while (true)
	{
		//  Get the next command. If there is none, exit.
		Command cmd;
		int rc = mailbox_.Recv(&cmd);
		if (rc != 0 && errno == EINTR)
			continue;
		if (rc != 0 && errno == EAGAIN)
			break;
		assert (rc == 0);
		//  Process the command.
		cmd.destination_->ProcessCommand(cmd);		
	}
}

void FREEZE_NET::IOThread::OutEvent()
{
	//  We are never polling for POLLOUT here. This function is never called.
	assert(false);
}

void FREEZE_NET::IOThread::TimerEvent(int id)
{
	//  No timers here. This function is never called.
	assert(false);
}

FREEZE_NET::Poller *FREEZE_NET::IOThread::GetPoller()
{
	assert(poller_ != NULL);
	return poller_;
}

void FREEZE_NET::IOThread::ProcessStop()
{
	poller_->RmFd(mailbox_handle_);
	poller_->Stop();
}
