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

#ifndef __SHARED_NET_CTX_H__
#define __SHARED_NET_CTX_H__

#include <map>
#include <vector>
#include <string>
#include <stdarg.h>
#include <stdint.h>

#include "Mailbox.h"
#include "Mutex.h"
#include "Thread.h"
#include "AtomicCounter.h"

#include "../Serialization/ByteBuffer.h"
#include "../Utils/SpinLock.h"

#include "../Serialization/ProtocolHead.h"
#include "../Serialization/Opcodes.h"

#include "../Utils/Singleton.h"

namespace FREEZE_NET
{
	//  Context object encapsulates all the global state associated with
	//  the library.

    class MySQLRPCServer;

	class Ctx
	{
	public:

		//  Create the context object. The argument specifies the size
		//  of I/O thread pool to create.
		Ctx(uint32_t io_threads = 16);

		//  Returns false if object is not a context.
		bool CheckTag();

		//  This function is called when user invokes zmq_term. If there are
		//  no more sockets open it'll cause all the infrastructure to be shut
		//  down. If there are open sockets still, the deallocation happens
		//  after the last one is closed.
		int Terminate();

		//  Send command to the destination thread.
		void SendCommand(uint32_t tid, const Command& command);

		//  Returns the I/O thread that is the least busy at the moment.
		//  Affinity specifies which I/O threads are eligible (0 = all).
		//  Returns NULL is no I/O thread is available.
		class IOThread* ChooseIOThread(uint64_t affinity);

        class MySQLRPCClient* MysqlRPCClient();
        MySQLRPCServer* MysqlRPCServer();
		MySQLRPCServer* MysqlRPCLog();

		class SocketBase* GetSocketBase();
		enum
		{
			term_tid = 0,
			rpc_mysql_client_tid = 1,
			rpc_mysql_server_tid = 2,
			rpc_mysql_log_tid    = 3,
			enum_count
		};

		~Ctx();

		uint32_t GenSerialNum(void);
		void RegisterZmqEngine(uint32_t serial_num, class ZmqEngine* ptr_engine);
		void UnregisterZmqEngine(uint32_t serial_num);
		void RegisterSendBuf(uint32_t serial_num);
		bool SendBuf(uint32_t serial_num, std::string buf, ProtocolHead protocol_haad);
		bool SendBuf(uint32_t serial_num, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad);
		std::string BuildPackage(ProtocolHead protocol_haad, unsigned char* ptr, uint32_t ptr_size);
		std::string DrainBuf(uint32_t serial_num);
		void Close(uint32_t serial_num);

		//void ActivateSendBuf(uint32_t serial_num);

		Opcodes& GetOpcodes(void);

		SpinLock& GetEngineMutex();
		std::map<uint32_t, class ZmqEngine*>& GetEngine();
	private:

		//  Used to check whether the object is a context.
		uint32_t tag_;

		//  If true, zmq_term was already called.
		bool terminating_;

		//  Array of pointers to mailboxes for both application and I/O threads.
		uint32_t slot_count_;
		Mailbox **slots_;

		//  I/O threads.
		typedef std::vector<class IOThread*> io_threads_t;
		io_threads_t io_threads_;

		typedef std::vector<uint32_t> emtpy_slots_t;
		emtpy_slots_t empty_slots_;

		//  Mailbox for zmq_term thread.
		Mailbox term_mailbox_;

		class SocketBase* container_ptr;

		SpinLock engine_mutex_;
		std::map<uint32_t, class ZmqEngine*> engine_;
		std::map<uint32_t, std::string> send_buf_;

		AtomicCounter gen_serial_num_;

		Opcodes opcodes_;

		Ctx(const Ctx&);
		const Ctx& operator=(const Ctx&);

	public:
        class MySQLRPCClient* mysql_rpc_client_;
        MySQLRPCServer* mysql_rpc_server_;
		MySQLRPCServer* mysql_rpc_log_;
	};

}

//using namespace FREEZE_NET;

#define SINGLETON_CTX Singleton<FREEZE_NET::Ctx>::Instance()

#endif

