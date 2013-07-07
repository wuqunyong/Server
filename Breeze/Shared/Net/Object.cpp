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

#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "Fd.h"
#include "Object.h"
#include "Ctx.h"
#include "IOThread.h"
#include "MySQLRPCClient.h"
#include "MySQLRPCServer.h"
#include "Own.h"

FREEZE_NET::Object::Object(Ctx* ctx, uint32_t tid) :
	ctx_(ctx), tid_(tid)
{
}

FREEZE_NET::Object::Object(Object* parent) :
	ctx_(parent->ctx_), tid_(parent->tid_)
{
}

FREEZE_NET::Object::~Object(void)
{
}

uint32_t FREEZE_NET::Object::GetTid(void)
{
	return tid_;
}

FREEZE_NET::Ctx *FREEZE_NET::Object::GetCtx(void)
{
	return ctx_;
}


void FREEZE_NET::Object::ProcessCommand(Command& cmd)
{
	switch (cmd.type_)
	{
	case Command::STOP:
		{
			ProcessStop();
			break;
		}
	case Command::RPC_MYSQL_REQUEST:
		{
			ProcessRPCMysqlRequest(cmd.args_.RPC_MYSQL_REQUEST.msg_id_,
				cmd.args_.RPC_MYSQL_REQUEST.table_,
				cmd.args_.RPC_MYSQL_REQUEST.query_statement_,
				cmd.args_.RPC_MYSQL_REQUEST.callback_cmd_,
				cmd.args_.RPC_MYSQL_REQUEST.query_result_,
				cmd.args_.RPC_MYSQL_REQUEST.request_object_);
			break;
		}
	case Command::RPC_MYSQL_RESPONSE:
		{
			ProcessRPCMysqlResponse(cmd.args_.RPC_MYSQL_RESPONSE.msg_id_,
				cmd.args_.RPC_MYSQL_RESPONSE.return_result_,
				cmd.args_.RPC_MYSQL_RESPONSE.query_result_);
			break;
		}
	case Command::PLUG:
		{
			ProcessPlug();
			ProcessSeqnum();
			return;
		}
	case Command::OWN:
		{
			ProcessOwn(cmd.args_.OWN.object_);
			ProcessSeqnum();
			break;
		}
	case Command::TERM_REQ:
		{
			ProcessTermReq(cmd.args_.TERM_REQ.object_);
			break;
		}
	case Command::TERM:
		{
			ProcessTerm(cmd.args_.TERM.linger_);
			break;
		}
	case Command::TERM_ACK:
		{
			ProcessTermAck();
			break;
		}
	case Command::RPC_MYSQL_NOTIFY:
		{
			ProcessRPCMysqlNotify(cmd.args_.RPC_MYSQL_NOTIFY.table_, cmd.args_.RPC_MYSQL_NOTIFY.query_statement_);
			break;
		}
	default:
		assert (false);
	}

	//  The assumption here is that each command is processed once only,
	//  so deallocating it after processing is all right.
	DeallocateCommand(&cmd);
}

FREEZE_NET::IOThread *FREEZE_NET::Object::ChooseIOThread(uint64_t affinity)
{
	return ctx_->ChooseIOThread(affinity);
}

void FREEZE_NET::Object::SendStop()
{
	//  'stop' command goes always from administrative thread to
	//  the current object.
	Command cmd;
	cmd.destination_ = this;
	cmd.type_ = Command::STOP;
	ctx_->SendCommand(tid_, cmd);
}

void FREEZE_NET::Object::SendRPCMysqlRequest(MySQLRPCServer* destination,
		uint32_t msg_id, const char* table, const char* query_statement,
		CommandBaseEx* callback_cmd,
		ResultSet* query_result,
		MySQLRPCClient* request_object)
{
	assert(NULL != callback_cmd);

	Command cmd;
	cmd.destination_ = destination;
	cmd.type_ = Command::RPC_MYSQL_REQUEST;

	cmd.args_.RPC_MYSQL_REQUEST.msg_id_ = msg_id;
	cmd.args_.RPC_MYSQL_REQUEST.table_ = table;
	cmd.args_.RPC_MYSQL_REQUEST.query_statement_ = query_statement;
	cmd.args_.RPC_MYSQL_REQUEST.callback_cmd_ = callback_cmd;
	cmd.args_.RPC_MYSQL_REQUEST.query_result_ = query_result;
	cmd.args_.RPC_MYSQL_REQUEST.request_object_ = request_object;

	SendCommand(cmd);
}

void FREEZE_NET::Object::SendRPCMysqlNotify(MySQLRPCServer* destination, const char* query_statement, const char* table)
{
	Command cmd;
	cmd.destination_ = destination;
	cmd.type_ = Command::RPC_MYSQL_NOTIFY;

	cmd.args_.RPC_MYSQL_NOTIFY.table_ = table;
	cmd.args_.RPC_MYSQL_NOTIFY.query_statement_ = query_statement;

	SendCommand(cmd);
}

void FREEZE_NET::Object::SendRPCMysqlResponse(MySQLRPCClient* destination, uint32_t msg_id,
		uint8_t return_result, ResultSet* query_result)
{
	Command cmd;
	cmd.destination_ = destination;
	cmd.type_ = Command::RPC_MYSQL_RESPONSE;

	cmd.args_.RPC_MYSQL_RESPONSE.msg_id_ = msg_id;
	cmd.args_.RPC_MYSQL_RESPONSE.return_result_ = return_result;
	cmd.args_.RPC_MYSQL_RESPONSE.query_result_ = query_result;

	SendCommand(cmd);
}



void FREEZE_NET::Object::SendPlug(class Own *destination, bool inc_seqnum)
{
	if (inc_seqnum)
		destination->IncSeqnum();

	Command cmd;
#if defined ZMQ_MAKE_VALGRIND_HAPPY
	memset (&cmd, 0, sizeof (cmd));
#endif
	cmd.destination_ = destination;
	cmd.type_ = Command::PLUG;
	SendCommand(cmd);
}

void FREEZE_NET::Object::SendActivateOut(class Own *destination, uint32_t serial_num)
{

	Command cmd;
#if defined ZMQ_MAKE_VALGRIND_HAPPY
	memset (&cmd, 0, sizeof (cmd));
#endif
	cmd.destination_ = destination;
	cmd.type_ = Command::ACTIVATE_OUT;
	cmd.args_.ACTIVATE_OUT.serial_num_ = serial_num;
	SendCommand(cmd);
}

void FREEZE_NET::Object::SendOwn(class Own *destination, class Own *object)
{
	destination->IncSeqnum();
	Command cmd;

#if defined ZMQ_MAKE_VALGRIND_HAPPY
	memset (&cmd, 0, sizeof (cmd));
#endif
	cmd.destination_ = destination;
	cmd.type_ = Command::OWN;
	cmd.args_.OWN.object_ = object;
	SendCommand(cmd);
}

void  FREEZE_NET::Object::SendTermReq(class Own *destination, class Own *object)
{
	Command cmd;
#if defined ZMQ_MAKE_VALGRIND_HAPPY
	memset (&cmd, 0, sizeof (cmd));
#endif
	cmd.destination_ = destination;
	cmd.type_ = Command::TERM_REQ;
	cmd.args_.TERM_REQ.object_ = object;
	SendCommand(cmd);
}

void FREEZE_NET::Object::SendTerm(Own *destination, int linger)
{
	Command cmd;
#if defined ZMQ_MAKE_VALGRIND_HAPPY
	memset (&cmd, 0, sizeof (cmd));
#endif
	cmd.destination_ = destination;
	cmd.type_ = Command::TERM;
	cmd.args_.TERM.linger_ = linger;
	SendCommand(cmd);
}

void  FREEZE_NET::Object::SendTermAck(class Own *destination)
{
	Command cmd;
#if defined ZMQ_MAKE_VALGRIND_HAPPY
	memset (&cmd, 0, sizeof (cmd));
#endif
	cmd.destination_ = destination;
	cmd.type_ = Command::TERM_ACK;
	SendCommand(cmd);
}



void FREEZE_NET::Object::ProcessStop()
{
	//assert(false);
}


void FREEZE_NET::Object::ProcessRPCMysqlRequest(uint32_t msg_id, const char* table,
		const char* query_statement, CommandBaseEx* callback_cmd,
		ResultSet* query_result,
		MySQLRPCClient* request_object)
{
	assert(false);
}

void FREEZE_NET::Object::ProcessRPCMysqlNotify(const char* table, const char* query_statement)
{
	assert(false);
}

void FREEZE_NET::Object::ProcessRPCMysqlResponse(uint32_t msg_id,
		uint8_t return_result, ResultSet* query_result)
{
	//assert(false);
}

void FREEZE_NET::Object::ProcessOwn(class Own *object)
{
	assert(false);
}
void  FREEZE_NET::Object::ProcessTermReq(class Own *object)
{
	assert(false);
}
void  FREEZE_NET::Object::ProcessTerm(int linger)
{
	assert(false);
}
void  FREEZE_NET::Object::ProcessTermAck()
{
	assert(false);
}

void FREEZE_NET::Object::ProcessActivateOut()
{
	assert(false);
}

void FREEZE_NET::Object::ProcessPlug()
{
	assert(false);
}

void FREEZE_NET::Object::ProcessSeqnum()
{
	assert(false);
}

void FREEZE_NET::Object::SendCommand(Command& cmd)
{
	ctx_->SendCommand(cmd.destination_->GetTid(), cmd);
}

