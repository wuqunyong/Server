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

#include <new>

#include "Fd.h"

#include <assert.h>

#if defined WIN32
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include "ZmqConnecter.h"
#include "ZmqEngine.h"
#include "IOThread.h"
//#include "ZmqInit.h"
#include "Ctx.h"

FREEZE_NET::ZmqConnecter::ZmqConnecter (class IOThread *io_thread, sockaddr_in addr, bool wait, uint32_t serial_num) :
    Own(io_thread),
    IOObject(io_thread),
    handle_valid_(false),
    wait_(wait),
    current_reconnect_ivl_(1000),
	serial_num_(serial_num)
{
    int rc = tcp_connecter_.SetAddress(addr);
    assert(rc == 0);
}

FREEZE_NET::ZmqConnecter::~ZmqConnecter ()
{
    if (wait_)
	{
		 CancelTimer(reconnect_timer_id);
	}
       
    if (handle_valid_)
	{
		RmFd(handle_);
	}
        
}

void FREEZE_NET::ZmqConnecter::ProcessPlug()
{
    if (wait_)
	{
		AddReconnectTimer();
	}
    else
	{
		 StartConnecting();
	}
}

void FREEZE_NET::ZmqConnecter::InEvent()
{
    //  We are not polling for incomming data, so we are actually called
    //  because of error here. However, we can get error on out event as well
    //  on some platforms, so we'll simply handle both events in the same way.
    OutEvent();
}

void FREEZE_NET::ZmqConnecter::OutEvent()
{
    fd_t fd = tcp_connecter_.Connect();
    RmFd(handle_);
    handle_valid_ = false;

    //  Handle the error condition by attempt to reconnect.
    if (fd == retired_fd) 
	{
        tcp_connecter_.Close();
        wait_ = true;
        AddReconnectTimer();
        return;
    }

    //  Choose I/O thread to run connecter in. Given that we are already
    //  running in an I/O thread, there must be at least one available.
    IOThread *io_thread = ChooseIOThread(0) ;
    assert(NULL != io_thread);

	//uint32_t serial_num = GetCtx()->GenSerialNum();

	std::cout << "Connect fd: " << fd << std::endl;

    //  Create an init object. 
	ZmqEngine *init = new (std::nothrow) ZmqEngine (io_thread, fd, this->serial_num_);
	assert(NULL != init);
	LaunchSibling(init);


    //  Shut the connecter down.
    Terminate();
}

void FREEZE_NET::ZmqConnecter::TimerEvent(int id)
{
    assert(id == reconnect_timer_id);
    wait_ = false;
    StartConnecting();
}

void FREEZE_NET::ZmqConnecter::StartConnecting()
{
    //  Open the connecting socket.
    int rc = tcp_connecter_.Open();

    //  Connect may succeed in synchronous manner.
    if (rc == 0) 
	{
        handle_ = AddFd(tcp_connecter_.GetFd());
        handle_valid_ = true;
        OutEvent();
        return;
    }

    //  Connection establishment may be dealyed. Poll for its completion.
    else if (rc == -1 && errno == EAGAIN)
	{
        handle_ = AddFd(tcp_connecter_.GetFd());
        handle_valid_ = true;
        SetPollout(handle_);
        return;
    }

    //  Handle any other error condition by eventual reconnect.
    wait_ = true;
    AddReconnectTimer();
}

void FREEZE_NET::ZmqConnecter::AddReconnectTimer()
{
    AddTimer(GetNewReconnectIvl(), reconnect_timer_id);
}

int FREEZE_NET::ZmqConnecter::GetNewReconnectIvl()
{
    return current_reconnect_ivl_;
}
