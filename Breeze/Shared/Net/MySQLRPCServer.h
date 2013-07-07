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

#ifndef __SHARED_NET_MYSQL_RPC_SERVER_H__
#define __SHARED_NET_MYSQL_RPC_SERVER_H__

#include <vector>
#include <map>
#include <stdint.h>

#include "Object.h"
#include "Poller.h"
#include "IPollEvents.h"
#include "Mailbox.h"

#include "../Db/MySQLConnection.h"

class CommandBaseEx;

namespace FREEZE_NET
{

	class MySQLRPCClient;
	class MySQLRPCServer: public Object, public IPollEvents
	{
	public:

		MySQLRPCServer(class Ctx* ctx, uint32_t tid);

		//  Clean-up. If the thread was started, it's neccessary to call 'stop'
		//  before invoking destructor. Otherwise the destructor would hang up.
		virtual ~MySQLRPCServer();

		//  Launch the physical thread.
		void Start();

		//  Ask underlying thread to stop.
		void Stop();

		//  Returns mailbox associated with this I/O thread.
		Mailbox* GetMailbox();

		//  i_poll_events implementation.
		void InEvent();
		void OutEvent();
		void TimerEvent(int id);

		//  Used by io_objects to retrieve the assciated poller object.
		Poller* GetPoller();

		//  Command handlers.
		virtual void ProcessStop();
		virtual void ProcessRPCMysqlRequest(uint32_t msg_id, const char* table,
				const char* query_statement, CommandBaseEx* callback_cmd,
				ResultSet* query_result,
				MySQLRPCClient* request_object);
		virtual void ProcessRPCMysqlNotify(const char* table, const char* query_statement);

		//  Returns load experienced by the I/O thread.
		int GetLoad();

		bool Connect(MySQLConnectionInfo conn_info);
	private:

		//  I/O thread accesses incoming commands via this mailbox.
		Mailbox mailbox_;

		//  Handle associated with mailbox' file descriptor.
		Poller::handle_t mailbox_handle_;

		//  I/O multiplexing is performed using a poller object.
		Poller* poller_;

		MySQLConnection mysql_connection_;


		MySQLRPCServer(const MySQLRPCServer&);
		const MySQLRPCServer& operator=(const MySQLRPCServer&);
	};

}
#endif
