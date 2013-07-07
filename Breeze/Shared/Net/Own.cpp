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

#include "Fd.h"
#include "Own.h"
#include "IOThread.h"

FREEZE_NET::Own::Own(class Ctx *parent, uint32_t tid) :
    Object(parent, tid),
    terminating_(false),
    sent_seqnum_(0),
    processed_seqnum_(0),
    owner_(NULL),
    term_acks_(0)
{
}

FREEZE_NET::Own::Own(IOThread *io_thread) :
    Object(io_thread),
    terminating_(false),
    sent_seqnum_(0),
    processed_seqnum_(0),
    owner_(NULL),
    term_acks_(0)
{
}

FREEZE_NET::Own::~Own()
{
}

void FREEZE_NET::Own::SetOwner(Own *owner)
{
    assert(!owner_);
    owner_ = owner;
}

FREEZE_NET::Own * FREEZE_NET::Own::GetOwner(void)
{
	return owner_;
}


void FREEZE_NET::Own::IncSeqnum()
{
    //  This function may be called from a different thread!
    sent_seqnum_.Add(1);
}

void FREEZE_NET::Own::ProcessSeqnum()
{
    //  Catch up with counter of processed commands.
    processed_seqnum_++;

    //  We may have catched up and still have pending terms acks.
    CheckTermAcks();
}

void FREEZE_NET::Own::LaunchChild(Own *object)
{
    //  Specify the owner of the object.
    object->SetOwner(this);

    //  Plug the object into the I/O thread.
    SendPlug(object);

    //  Take ownership of the object.
    SendOwn(this, object);
}

void FREEZE_NET::Own::LaunchSibling(Own *object)
{
    //  At this point it is important that object is plugged in before its
    //  owner has a chance to terminate it. Thus, 'plug' command is sent before
    //  the 'own' command. Given that the mailbox preserves ordering of
    //  commands, 'term' command from the owner cannot make it to the object
    //  before the already written 'plug' command.

    //  Specify the owner of the object.
    object->SetOwner(owner_);

    //  Plug the object into its I/O thread.
    SendPlug(object);

    //  Make parent own the object.
    SendOwn(owner_, object);
}

void FREEZE_NET::Own::ProcessTermReq(Own *object)
{
    //  When shutting down we can ignore termination requests from owned
    //  objects. The termination request was already sent to the object.
    if (terminating_)
	{
		return;
	}
        

    //  If I/O object is well and alive let's ask it to terminate.
    owned_t::iterator it = std::find(owned_.begin(), owned_.end(), object);

    //  If not found, we assume that termination request was already sent to
    //  the object so we can safely ignore the request.
    if (it == owned_.end ())
	{
		 return;
	}
       

    owned_.erase (it);
    RegisterTermAcks(1);

    //  Note that this object is the root of the (partial shutdown) thus, its
    //  value of linger is used, rather than the value stored by the children.
    SendTerm(object, 0);
}

void FREEZE_NET::Own::ProcessOwn(Own *object)
{
    //  If the object is already being shut down, new owned objects are
    //  immediately asked to terminate. Note that linger is set to zero.
    if (terminating_) 
	{
        RegisterTermAcks(1);
        SendTerm(object, 0);
        return;
    }

    //  Store the reference to the owned object.
    owned_.insert(object);
}

void FREEZE_NET::Own::Terminate()
{
    //  If termination is already underway, there's no point
    //  in starting it anew.
    if (terminating_)
	{
		return;
	}
        

    //  As for the root of the ownership tree, there's noone to terminate it,
    //  so it has to terminate itself.
    if (!owner_)
	{
        ProcessTerm(0);
        return;
    }

    //  If I am an owned object, I'll ask my owner to terminate me.
    SendTermReq(owner_, this);
}

void FREEZE_NET::Own::ProcessTerm(int linger)
{
    //  Double termination should never happen.
    assert(!terminating_);

    //  Send termination request to all owned objects.
    for (owned_t::iterator it = owned_.begin (); it != owned_.end (); ++it)
	{
		 SendTerm(*it, linger);
	}
       
    RegisterTermAcks(owned_.size());
    owned_.clear();

    //  Start termination process and check whether by chance we cannot
    //  terminate immediately.
    terminating_ = true;
    CheckTermAcks();
}

void FREEZE_NET::Own::RegisterTermAcks(int count_)
{
    term_acks_ += count_;
}

void FREEZE_NET::Own::UnregisterTermAck()
{
    assert(term_acks_ > 0);
    term_acks_--;

    //  This may be a last ack we are waiting for before termination...
    CheckTermAcks(); 
}

void FREEZE_NET::Own::ProcessTermAck()
{
    UnregisterTermAck();
}

void FREEZE_NET::Own::CheckTermAcks()
{
    if (terminating_ && processed_seqnum_ == sent_seqnum_.Get() && term_acks_ == 0)
	{

        //  Sanity check. There should be no active children at this point.
        assert(owned_.empty());

        //  The root object has nobody to confirm the termination to.
        //  Other nodes will confirm the termination to the owner.
        if (owner_)
		{
			SendTermAck(owner_);
		}
            

        //  Deallocate the resources.
        ProcessDestroy();
    }
}

void FREEZE_NET::Own::ProcessDestroy()
{
    delete this;
}

