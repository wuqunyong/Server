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

#ifndef __SHARED_NET_MYSQL_RPC_CLIENT_H__
#define __SHARED_NET_MYSQL_RPC_CLIENT_H__

#include <vector>
#include <map>
#include <stdint.h>

#include "Object.h"
#include "Poller.h"
#include "IPollEvents.h"
#include "Mailbox.h"
#include "Mutex.h"

#include "../Db/ResultSet.h"
#include "../Utils/CommandBase.h"
#include "../Utils/SpinLock.h"
#include "../Utils/Guard.h"

#include "../Serialization/ByteBuffer.h"

namespace FREEZE_NET
{
	class Ctx;
	class MySQLRPCServer;

	//单个连接的消息队列
	class SessionMsgQueue
	{
	public:
		SessionMsgQueue();
		~SessionMsgQueue();

		void SetSerialNum(uint32_t serial_num);
		uint32_t GetSerialNum(void);

		void SetState(uint32_t val);
		uint32_t GetState(void);

		std::queue<ByteBuffer *>& GetMsgQueue();

		enum SESSION_STATE
		{
			RUNNING = 0,
			BLOCKED,
			ENUM_COUNT
		};
	private:
		uint32_t serial_num_;
		uint32_t state_;
		std::queue<ByteBuffer *> msg_queue_;
	};

	class MySQLRPCClient: public Object, public IPollEvents
	{
	public:

		MySQLRPCClient(Ctx* ctx, uint32_t tid);

		//  Clean-up. If the thread was started, it's neccessary to call 'stop'
		//  before invoking destructor. Otherwise the destructor would hang up.
		virtual ~MySQLRPCClient();

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
		void ProcessStop();

		//  Returns load experienced by the I/O thread.
		int GetLoad();

		//set_session_state: 回调函数调用时重新设置该SESSION的状态
		void SendRPC(char const* query_statement,
				CommandBaseEx* callback_cmd,
				uint32_t serial_num,
				uint32_t set_session_state = SessionMsgQueue::RUNNING,
				char const* table = "");
		void SendRPCNotify(char const* query_statement, char const* table = "");
		void SendRPCNotifyLog(char const* query_statement, char const* table = "");

		void ProcessRPCMysqlResponse(uint32_t msg_id, uint8_t return_result,
				ResultSet* query_result);

		void AddStubs(uint32_t msg_id, Command& cmd);
		bool FindStubs(uint32_t msg_id, Command& cmd);
		void RemoveStubs(uint32_t msg_id);

	public:

		void HandlePRCResponse(ResultSet* query_result);

		//生产者往producer 推送数据包
		void Push(uint32_t serial_num, ByteBuffer* ptr_value);
		ByteBuffer* Pop(uint32_t serial_num);

		uint32_t GetSessionState(uint32_t serial_num);
		void SetSessionState(uint32_t serial_num, uint32_t state);

	private:
		void Swap(void);

		void TransferPush(uint32_t serial_num);
		void DestroySession(uint32_t serial_num);
	private:

		//  I/O thread accesses incoming commands via this mailbox.
		Mailbox mailbox_;

		//  Handle associated with mailbox' file descriptor.
		Poller::handle_t mailbox_handle_;

		//  I/O multiplexing is performed using a poller object.
		Poller* poller_;

		//cmd.args_.RPC_MYSQL_REQUEST.query_result_ 保存的为NULL指针
		std::map<uint32_t, Command> client_stubs_;

		AtomicCounter msg_id_;
		SpinLock lock_;

		std::queue<uint32_t>* producer_;
		std::queue<uint32_t>* consumer_;

		std::map<uint32_t, SessionMsgQueue> session_map_;

		MySQLRPCClient(const MySQLRPCClient&);
		const MySQLRPCClient& operator=(const MySQLRPCClient&);
	};

}
#endif
