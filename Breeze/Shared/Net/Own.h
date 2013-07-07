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

#ifndef __SHARED_NET_OWN_H__
#define __SHARED_NET_OWN_H__

#include <set>
#include <algorithm>

#include "Object.h"
#include "AtomicCounter.h"

namespace FREEZE_NET
{

    //  Base class for objects forming a part of ownership hierarchy.
    //  It handles initialisation and destruction of such objects.

    class Own : public Object
    {
    public:

        //  Note that the owner is unspecified in the constructor.
        //  It'll be supplied later on when the object is plugged in.

        //  The object is not living within an I/O thread. It has it's own
        //  thread outside of 0MQ infrastructure.
        Own(class Ctx *parent, uint32_t tid);

        //  The object is living within I/O thread.
        Own(class IOThread *io_thread);

        //  When another owned object wants to send command to this object
        //  it calls this function to let it know it should not shut down
        //  before the command is delivered.
        void IncSeqnum();

        //  Use following two functions to wait for arbitrary events before
        //  terminating. Just add number of events to wait for using
        //  register_tem_acks functions. When event occurs, call
        //  remove_term_ack. When number of pending acks reaches zero
        //  object will be deallocated.
        void RegisterTermAcks(int count);
        void UnregisterTermAck();

		Own * GetOwner(void);
    protected:

        //  Launch the supplied object and become its owner.
        void LaunchChild(Own *object);

        //  Launch the supplied object and make it your sibling (make your
        //  owner become its owner as well).
        void LaunchSibling(Own *object);

        //  Ask owner object to terminate this object. It may take a while
        //  while actual termination is started. This function should not be
        //  called more than once.
        void Terminate();

        //  Derived object destroys Own. There's no point in allowing
        //  others to invoke the destructor. At the same time, it has to be
        //  virtual so that generic Own deallocation mechanism destroys
        //  specific type of the owned object correctly.
        virtual ~Own();

        //  Term handler is protocted rather than private so that it can
        //  be intercepted by the derived class. This is useful to add custom
        //  steps to the beginning of the termination process.
        void ProcessTerm(int linger);

        //  A place to hook in when phyicallal destruction of the object
        //  is to be delayed.
        virtual void ProcessDestroy();

    private:

        //  Set owner of the object
        void SetOwner(Own *owner);

        //  Handlers for incoming commands.
        void ProcessOwn(Own *object);
        void ProcessTermReq(Own *object);
        void ProcessTermAck();
        void ProcessSeqnum();

        //  Check whether all the peding term acks were delivered.
        //  If so, deallocate this object.
        void CheckTermAcks();

        //  True if termination was already initiated. If so, we can destroy
        //  the object if there are no more child objects or pending term acks.
        bool terminating_;

        //  Sequence number of the last command sent to this object.
        AtomicCounter sent_seqnum_;

        //  Sequence number of the last command processed by this object.
        uint64_t processed_seqnum_;

        //  Socket owning this object. It's responsible for shutting down
        //  this object.
        Own *owner_;

        //  List of all objects owned by this socket. We are responsible
        //  for deallocating them before we quit.
        typedef std::set <Own*> owned_t;
        owned_t owned_;

        //  Number of events we have to get before we can destroy the object.
        int term_acks_;

        Own (const Own&);
        const Own &operator = (const Own&);
    };

}

#endif
