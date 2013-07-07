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
#include "Epoll.h"

#ifndef WIN32

#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <algorithm>
#include <new>
#include <assert.h>

#include "IPollEvents.h"

FREEZE_NET::Epoll::Epoll() :
	stopping_(false)
{
	epoll_fd_ = epoll_create(1);
	assert(epoll_fd_ != -1);
}

FREEZE_NET::Epoll::~Epoll()
{
	//  Wait till the worker thread exits.
	worker_.Stop();

	close(epoll_fd_);
	for (retired_t::iterator it = retired_.begin(); it != retired_.end(); ++it)
	{
		delete *it;
	}

}

FREEZE_NET::Epoll::handle_t FREEZE_NET::Epoll::AddFd(fd_t fd, IPollEvents* events)
{
	poll_entry_t *pe = new (std::nothrow) poll_entry_t;
	assert(pe != NULL);

	//  The memset is not actually needed. It's here to prevent debugging
	//  tools to complain about using uninitialised memory.
	memset(pe, 0, sizeof(poll_entry_t));

	pe->fd_ = fd;
	pe->ev_.events = 0;
	pe->ev_.data.ptr = pe;
	pe->events_ = events;

	int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &pe->ev_);
	assert(rc != -1);

	//  Increase the load metric of the thread.
	AdjustLoad(1);

	return pe;
}

void FREEZE_NET::Epoll::RmFd(handle_t handle)
{
	poll_entry_t *pe = (poll_entry_t*) handle;
	int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, pe->fd_, &pe->ev_);
	assert(rc != -1);
	pe->fd_ = retired_fd;
	retired_.push_back(pe);

	//  Decrease the load metric of the thread.
	AdjustLoad(-1);
}

void FREEZE_NET::Epoll::SetPollin(handle_t handle)
{
	poll_entry_t *pe = (poll_entry_t*) handle;
	pe->ev_.events |= EPOLLIN;
	int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, pe->fd_, &pe->ev_);
	assert(rc != -1);
}

void FREEZE_NET::Epoll::ResetPollin(handle_t handle)
{
	poll_entry_t *pe = (poll_entry_t*) handle;
	pe->ev_.events &= ~((short) EPOLLIN);
	int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, pe->fd_, &pe->ev_);
	assert(rc != -1);
}

void FREEZE_NET::Epoll::SetPollout(handle_t handle)
{
	poll_entry_t *pe = (poll_entry_t*) handle;
	pe->ev_.events |= EPOLLOUT;
	int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, pe->fd_, &pe->ev_);
	assert(rc != -1);
}

void FREEZE_NET::Epoll::ResetPollout(handle_t handle)
{
	poll_entry_t *pe = (poll_entry_t*) handle;
	pe->ev_.events &= ~((short) EPOLLOUT);
	int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, pe->fd_, &pe->ev_);
	assert(rc != -1);
}

void FREEZE_NET::Epoll::Start()
{
	worker_.Start(worker_routine, this);
}

void FREEZE_NET::Epoll::Stop()
{
	stopping_ = true;
}

void FREEZE_NET::Epoll::Loop()
{
	epoll_event ev_buf[256];

	while (!stopping_)
	{

		//  Execute any due timers.
		int timeout = (int) ExecuteTimers();

		//  Wait for events.
		int n = epoll_wait(epoll_fd_, &ev_buf[0], 256, timeout ? timeout : -1);
		if (n == -1 && errno == EINTR)
		{
			continue;
		}

		assert(n != -1);

		for (int i = 0; i < n; i++)
		{
			poll_entry_t *pe = ((poll_entry_t*) ev_buf[i].data.ptr);

			if (pe->fd_ == retired_fd)
			{
				continue;
			}
			if (ev_buf[i].events & (EPOLLERR | EPOLLHUP))
			{
				pe->events_->InEvent();
			}

			if (pe->fd_ == retired_fd)
			{
				continue;
			}
			if (ev_buf[i].events & EPOLLOUT)
			{
				pe->events_->OutEvent();
			}

			if (pe->fd_ == retired_fd)
			{
				continue;
			}
			if (ev_buf[i].events & EPOLLIN)
			{
				pe->events_->InEvent();
			}

		}

		//  Destroy retired event sources.
		for (retired_t::iterator it = retired_.begin(); it != retired_.end(); ++it)
		{
			delete *it;
		}

		retired_.clear();

		usleep((10)*1000);
	}
}

void FREEZE_NET::Epoll::worker_routine(void *arg)
{
	((Epoll*) arg)->Loop();
}

#endif