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

#if defined WIN32
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#else
#include <sys/select.h>
#include <unistd.h>
#endif


#include <string.h>
#include <algorithm>

#include "Select.h"
#include "IPollEvents.h"

FREEZE_NET::Select::Select() :
    maxfd_(retired_fd),
    retired_(false),
    stopping_(false)
{
    //  Clear file descriptor sets.
    FD_ZERO(&source_set_in_);
    FD_ZERO(&source_set_out_);
    FD_ZERO(&source_set_err_);
}

FREEZE_NET::Select::~Select()
{
    worker_.Stop();
}

FREEZE_NET::Select::handle_t FREEZE_NET::Select::AddFd(fd_t fd, IPollEvents *events)
{
    //  Store the file descriptor.
    fd_entry_t entry = {fd, events};
    fds_.push_back(entry);

    //  Ensure we do not attempt to select () on more than FD_SETSIZE
    //  file descriptors.
    assert(fds_.size () <= FD_SETSIZE);

    //  Start polling on errors.
    FD_SET(fd, &source_set_err_);

    //  Adjust maxfd if necessary.
    if (fd > maxfd_)
        maxfd_ = fd;

    //  Increase the load metric of the thread.
    AdjustLoad(1);

    return fd;
}

void FREEZE_NET::Select::RmFd(handle_t handle)
{
    //  Mark the descriptor as retired.
    fd_set_t::iterator it;
    for (it = fds_.begin (); it != fds_.end (); ++it)
	{
		if (it->fd_ == handle)
		{
			break;
		}
	}
    assert(it != fds_.end());
    it->fd_ = retired_fd;
    retired_ = true;

    //  Stop polling on the descriptor.
    FD_CLR(handle, &source_set_in_);
    FD_CLR(handle, &source_set_out_);
    FD_CLR(handle, &source_set_err_);

    //  Discard all events generated on this file descriptor.
    FD_CLR(handle, &readfds_);
    FD_CLR(handle, &writefds_);
    FD_CLR(handle, &exceptfds_);

    //  Adjust the maxfd attribute if we have removed the
    //  highest-numbered file descriptor.
    if (handle == maxfd_) 
	{
        maxfd_ = retired_fd;
        for (fd_set_t::iterator it = fds_.begin (); it != fds_.end (); ++it)
		{
			if (it->fd_ > maxfd_)
			{
				maxfd_ = it->fd_;
			}
		}
    }

    //  Decrease the load metric of the thread.
    AdjustLoad(-1);
}

void FREEZE_NET::Select::SetPollin(handle_t handle)
{
    FD_SET(handle, &source_set_in_);
}

void FREEZE_NET::Select::ResetPollin(handle_t handle)
{
    FD_CLR(handle, &source_set_in_);
}

void FREEZE_NET::Select::SetPollout(handle_t handle)
{
    FD_SET(handle, &source_set_out_);
}

void FREEZE_NET::Select::ResetPollout(handle_t handle)
{
    FD_CLR(handle, &source_set_out_);
}

void FREEZE_NET::Select::Start()
{
    worker_.Start(worker_routine, this);
}

void FREEZE_NET::Select::Stop()
{
    stopping_ = true;
}

void FREEZE_NET::Select::Loop()
{
    while (!stopping_) 
	{

        //  Execute any due timers.
        int timeout = (int) ExecuteTimers();

        //  Intialise the pollsets.
        memcpy(&readfds_, &source_set_in_, sizeof source_set_in_);
        memcpy(&writefds_, &source_set_out_, sizeof source_set_out_);
        memcpy(&exceptfds_, &source_set_err_, sizeof source_set_err_);

        //  Wait for events.
        struct timeval tv = {(long) (timeout / 1000), (long) (timeout % 1000 * 1000)};
        int rc = select (maxfd_ + 1, &readfds_, &writefds_, &exceptfds_, timeout ? &tv : NULL);

#ifdef WIN32
        assert(rc != SOCKET_ERROR);
#else
        if (rc == -1 && errno == EINTR)
            continue;
        assert(rc != -1);
#endif

        //  If there are no events (i.e. it's a timeout) there's no point
        //  in checking the pollset.
        if (rc == 0)
            continue;

        for (fd_set_t::size_type i = 0; i < fds_.size (); i ++) 
		{
            if (fds_[i].fd_ == retired_fd)
                continue;
            if (FD_ISSET(fds_[i].fd_, &exceptfds_))
                fds_[i].events_->InEvent();
            if (fds_[i].fd_ == retired_fd)
                continue;
            if (FD_ISSET(fds_[i].fd_, &writefds_))
                fds_[i].events_->OutEvent();
            if (fds_[i].fd_ == retired_fd)
                continue;
            if (FD_ISSET(fds_[i].fd_, &readfds_))
                fds_[i].events_->InEvent();
        }

        //  Destroy retired event sources.
        if (retired_) 
		{
            fds_.erase (std::remove_if(fds_.begin (), fds_.end (), FREEZE_NET::Select::is_retired_fd), fds_.end ());
            retired_ = false;
        }

#ifdef WIN32
		Sleep(10);
#else
		usleep((10)*1000);
#endif
    }
}

void FREEZE_NET::Select::worker_routine(void *arg)
{
    ((Select*) arg)->Loop();
}

bool FREEZE_NET::Select::is_retired_fd (const fd_entry_t &entry)
{
    return (entry.fd_ == retired_fd);
}