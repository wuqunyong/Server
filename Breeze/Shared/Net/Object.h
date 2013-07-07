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

#ifndef __SHARED_NET_OBJECT_H__
#define __SHARED_NET_OBJECT_H__

#include <stdint.h>

#include "../Utils/CommandBase.h"
#include "../Db/MySQLConnection.h"


namespace FREEZE_NET
{
	//  Base class for all objects that participate in inter-thread
	//  communication.

	class Ctx;
	class MySQLRPCClient;
	class MySQLRPCServer;

	class Object
	{
	public:

		Object(Ctx* ctx, uint32_t tid);
		Object(Object* parent);
		virtual ~Object();

		uint32_t GetTid();
		Ctx *GetCtx();
		void ProcessCommand(struct Command& cmd);

	protected:

		//  Chooses least loaded I/O thread.
		class IOThread *ChooseIOThread(uint64_t affinity);

		//  Derived object can use these functions to send commands
		//  to other objects.
		void SendStop();

		void SendRPCMysqlRequest(MySQLRPCServer* destination,
				uint32_t msg_id, const char* table, const char* query_statement,
				CommandBaseEx* callback_cmd,
				ResultSet* query_result,
				MySQLRPCClient* request_object);

		void SendRPCMysqlNotify(MySQLRPCServer* destination, const char* query_statement, const char* table);

		void SendRPCMysqlResponse(class MySQLRPCClient* destination,
				uint32_t msg_id, uint8_t return_result,
				ResultSet* query_result);
		void SendPlug(class Own *destination, bool inc_seqnum = true);
		void SendOwn(class Own *destination, class Own *object);
		
		void SendTermReq(class Own *destination, class Own *object);
		void SendTerm(class Own *destination, int linger);
		void SendTermAck(class Own *destination);
		void SendActivateOut(class Own *destination, uint32_t serial_num);

		//  These handlers can be overloaded by the derived objects. They are
		//  called when command arrives from another thread.
		virtual void ProcessStop();
		virtual void ProcessRPCMysqlRequest(uint32_t msg_id, const char* table,
				const char* query_statement, CommandBaseEx* callback_cmd,
				ResultSet* query_result,
				MySQLRPCClient* request_object);

		virtual void ProcessRPCMysqlNotify(const char* table, const char* query_statement);

		virtual void ProcessRPCMysqlResponse(uint32_t msg_id, uint8_t return_result,
				ResultSet* query_result);

		virtual void ProcessOwn(class Own *object);
		virtual void ProcessTermReq(class Own *object);
		virtual void ProcessTerm(int linger);
		virtual void ProcessTermAck();
		virtual void ProcessPlug();
		virtual void ProcessActivateOut();

		//  Special handler called after a command that requires a seqnum
		//  was processed. The implementation should catch up with its counter
		//  of processed commands here.
		virtual void ProcessSeqnum();

	private:

		//  Context provides access to the global state.
		Ctx* ctx_;

		//  Thread ID of the thread the object belongs to.
		uint32_t tid_;

		void SendCommand(Command& cmd);

		Object(const Object&);
		const Object& operator=(const Object&);
	};

}

#endif
