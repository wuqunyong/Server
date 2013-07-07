#ifndef __NETSERVER_NETSERVER_H__
#define __NETSERVER_NETSERVER_H__

#include "Net/Fd.h"
#include "Utils/CommandBase.h"

#include "Db/ResultSet.h"
#include "Net/Ctx.h"
#include "Net/MySQLRPCClient.h"
#include "Net/MySQLRPCServer.h"
#include "Net/IOThread.h"
#include "Net/SocketBase.h"

#include "Serialization/ByteBuffer.h"
#include "Serialization/Opcodes.h"
#include "Serialization/ProtocolHead.h"

class ServerFacade : public IFacade
{
public:
	ServerFacade(void){}
	virtual ~ServerFacade(void){}

public:
	virtual void Initialize(void){}
	virtual void Update(void){}

public:
	std::map<uint32_t, IDelegate*> GenOpcodes(void)
	{
		std::map<uint32_t, IDelegate*> temp;

		::RegisterOpcodes(temp, 0, this, &ServerFacade::HandleEcho);

		return temp;
	}
public:
	void HandleEcho(ByteBuffer* ptr)
	{
		ProtocolHead temp_protocol_head;

		(*ptr) >> temp_protocol_head;
		uint32_t temp_len;
		(*ptr) >> temp_len;
		std::string send_data;
		(*ptr) >> send_data;
		std::cout << "Server ZmqEngine recv: " << send_data << std::endl;

		ByteBuffer buffer;

		uint32_t packet_len = 14;
		std::string data = "12345678";
		buffer << packet_len;
		buffer << data;

		temp_protocol_head.identification_++;
		SINGLETON_CTX->SendBuf(temp_protocol_head.cur_session_serial_num_, buffer.RdPos(), buffer.Length(), temp_protocol_head);
	}
};

class UnitTestNetServer
{
public:
	UnitTestNetServer(void);
	~UnitTestNetServer(void);

public:
	int Run(void);

};


#endif
