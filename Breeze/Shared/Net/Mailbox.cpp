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


#ifdef WIN32
#else
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#endif

#include "Fd.h"
#include "Mailbox.h"

FREEZE_NET::Mailbox::Mailbox(void)
{
}

FREEZE_NET::Mailbox::~Mailbox(void)
{
}

FREEZE_NET::fd_t FREEZE_NET::Mailbox::GetFd(void)
{
	return signaler_.GetFd();
}

void FREEZE_NET::Mailbox::Send(const Command& cmd)
{
	sync_.Lock();
	this->cpipe_.push_back(cmd);
	sync_.Unlock();

	signaler_.Send();
}

int FREEZE_NET::Mailbox::Recv(Command* cmd)
{
	signaler_.Recv();

	sync_.Lock();
	if (!this->cpipe_.empty())
	{
		*cmd = this->cpipe_.front();
		this->cpipe_.pop_front();

		sync_.Unlock();
		return 0;
	}

	errno = EAGAIN;
	sync_.Unlock();
	return -1;
}

