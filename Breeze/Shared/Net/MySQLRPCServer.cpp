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
#include "MySQLRPCServer.h"
#include "Ctx.h"

FREEZE_NET::MySQLRPCServer::MySQLRPCServer(Ctx* ctx, uint32_t tid) :
	Object(ctx, tid)
{
	//MySQLConnectionInfo conn_info;
	//conn_info.host = "127.0.0.1";
	//conn_info.user = "root";
	//conn_info.password = "123456";
	//conn_info.database = "test";
	//conn_info.port = 3306;

	//mysql_connection_.Initialize(conn_info);

	//bool result = mysql_connection_.Connect();
	//if (!result)
	//{
	//	std::cout << "mysql connect error !" << std::endl;
	//}

	poller_ = new (std::nothrow) Poller;
	assert(poller_ != NULL);

	mailbox_handle_ = poller_->AddFd(mailbox_.GetFd(), this);
	poller_->SetPollin(mailbox_handle_);

}

FREEZE_NET::MySQLRPCServer::~MySQLRPCServer()
{
	delete poller_;
}

bool FREEZE_NET::MySQLRPCServer::Connect(MySQLConnectionInfo conn_info)
{
	//MySQLConnectionInfo conn_info;
	//conn_info.host = "127.0.0.1";
	//conn_info.user = "root";
	//conn_info.password = "123456";
	//conn_info.database = "test";
	//conn_info.port = 3306;

	mysql_connection_.Initialize(conn_info);

	bool result = mysql_connection_.Connect();
	if (!result)
	{
		std::cout << "mysql connect error !" << std::endl;
	}

	return result;
}

void FREEZE_NET::MySQLRPCServer::Start()
{
	//  Start the underlying I/O thread.
	poller_->Start();
}

void FREEZE_NET::MySQLRPCServer::Stop()
{
	SendStop();
}

FREEZE_NET::Mailbox *FREEZE_NET::MySQLRPCServer::GetMailbox()
{
	return &mailbox_;
}

int FREEZE_NET::MySQLRPCServer::GetLoad()
{
	return poller_->GetLoad();
}

void FREEZE_NET::MySQLRPCServer::InEvent()
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

void FREEZE_NET::MySQLRPCServer::OutEvent()
{
	//  We are never polling for POLLOUT here. This function is never called.
	assert(false);
}

void FREEZE_NET::MySQLRPCServer::TimerEvent(int id)
{
	//  No timers here. This function is never called.
	assert(false);
}

FREEZE_NET::Poller *FREEZE_NET::MySQLRPCServer::GetPoller()
{
	assert(poller_ != NULL);
	return poller_;
}

void FREEZE_NET::MySQLRPCServer::ProcessStop()
{
	poller_->RmFd(mailbox_handle_);
	poller_->Stop();
}


void FREEZE_NET::MySQLRPCServer::ProcessRPCMysqlRequest(uint32_t msg_id, const char* table,
				const char* query_statement, CommandBaseEx* callback_cmd,
				ResultSet* query_result,
				MySQLRPCClient* request_object)
{
	assert(query_result == NULL);
	uint32_t result = mysql_connection_.Query(query_statement, query_result);

	SendRPCMysqlResponse(request_object, msg_id, result, query_result);
}

void FREEZE_NET::MySQLRPCServer::ProcessRPCMysqlNotify(const char* table, const char* query_statement)
{
	ResultSet* query_result = NULL;
	bool result = mysql_connection_.Query(query_statement, query_result);

	if (!result)
	{
		std::cout << "execute error : " << query_statement << std::endl;
	}
	
	if (query_result != NULL)
	{
		std::cout << " ProcessRPCMysqlNotify : " << "²»Ó¦°üº¬ SELECT Óï¾ä" << std::endl;
		delete query_result;
		query_result = NULL;
	}
	
}