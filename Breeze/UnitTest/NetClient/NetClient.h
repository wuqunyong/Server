#ifndef __NETSERVER_NETCLIENT_H__
#define __NETSERVER_NETCLIENT_H__

#include "Net/Fd.h"
#include "Utils/CommandBase.h"

#include "Db/ResultSet.h"
#include "Net/Ctx.h"
#include "Net/MySQLRPCClient.h"
#include "Net/MySQLRPCServer.h"
#include "Net/IOThread.h"
#include "Net/SocketBase.h"
#include "Net/IPollEvents.h"

#include "Serialization/ByteBuffer.h"
#include "Serialization/Opcodes.h"
#include "Serialization/ProtocolHead.h"

#include "Game/NetNodeId.h"

class TestTimer : public FREEZE_NET::IPollEvents
{
public:

	void InEvent(void){};
	void OutEvent(void){};

	void TimerEvent(int id)
	{
		std::cout << "TimerEvent " << id << std::endl;
	};
};

class TestFacade : public IFacade
{
public:
	TestFacade(void){}
	virtual ~TestFacade(void){}

public:
	virtual void Initialize(void){}
	virtual void Update(void){}

public:
	std::map<uint32_t, IDelegate*> GenOpcodes(void)
	{
		std::map<uint32_t, IDelegate*> temp;

		::RegisterOpcodes(temp, Opcodes::ECHO, this, &TestFacade::HandleEcho);
		::RegisterOpcodes(temp, Opcodes::CLOSE_SESSION, this, &TestFacade::HandleColseSession);
		::RegisterOpcodes(temp, Opcodes::ROUTING, this, &TestFacade::HandleRoutingSession);

		
		::RegisterOpcodes(temp, Opcodes::RESPONSE_TEST, this, &TestFacade::HandleResponse);
		::RegisterOpcodes(temp, Opcodes::NOTIFY_TEST, this, &TestFacade::HandleNotify);
		::RegisterOpcodes(temp, Opcodes::PLATFORM_ACCOUNT_LOGIN_RESPONSE, this, &TestFacade::HandlePlatformAccountLoginResponse);

		::RegisterOpcodes(temp, Opcodes::S_GET_ROLE_RESPONSE, this, &TestFacade::HandleGetRoleResponse);
		::RegisterOpcodes(temp, Opcodes::S_GATEWAY_ADDRESS_NOTIFY, this, &TestFacade::HandleGatewayAddressNotify);

		
		return temp;
	}

	//std::map<uint32_t, IDelegate*> GenForwardOpcodes(void)
	//{
	//	std::map<uint32_t, IDelegate*> temp;
	//	return temp;
	//}

public:
	void HandleGetRoleResponse(ByteBuffer* ptr)
	{
		ProtocolHead head;
		(*ptr) >> head; 

		uint32_t role_count = 0;
		(*ptr) >> role_count;
		std::cout << "role_count : " << role_count << std::endl;


		for (uint32_t i = 0; i < role_count; i++)
		{
			uint32_t player_id = 0;
			(*ptr) >> player_id;

			std::cout << "player_id : " << player_id << std::endl;
		}
		
	}

	void HandleGatewayAddressNotify(ByteBuffer* ptr)
	{
		ProtocolHead head;
		(*ptr) >> head; 

		NetNodeAddress node_address;
		(*ptr) >> node_address;
		std::cout << "node_address : " << node_address.ip_ << " " << node_address.port_ << std::endl; 

		std::string validate;
		(*ptr) >> validate;
		std::cout << "validate: " << validate << std::endl;

		uint32_t role_count = 0;
		(*ptr) >> role_count;
		std::cout << "role_count: " << role_count << std::endl;

		std::set<uint32_t> role_id_set;
		for (uint32_t i = 0; i < role_count; i++)
		{
			uint32_t role_id = 0;
			(*ptr) >> role_id;
			std::cout << "role_id: " << role_id << std::endl;

			role_id_set.insert(role_id);
		}
	} 

	void HandlePlatformAccountLoginResponse(ByteBuffer* ptr)
	{

		ProtocolHead head;
		(*ptr) >> head; 

		uint32_t serial_num = head.suspend_session_serial_num_;

		uint32_t error_code = 0;
		(*ptr) >> error_code;

		std::cout << "error_code : " << error_code << std::endl;

		if (error_code == 0)
		{
			ByteBuffer request_buffer;

			ProtocolHead request_head;
			request_head.opcode_ = Opcodes::C_GET_ROLE_REQUEST;


			SINGLETON_CTX->SendBuf(serial_num, request_buffer.RdPos(), request_buffer.Length(), request_head);
		}
	}


	void HandleResponse(ByteBuffer* ptr)
	{

		ProtocolHead head;
		(*ptr) >> head; 

		std::string content;
		(*ptr) >> content;

		std::cout << content << std::endl;
	}

	void HandleNotify(ByteBuffer* ptr)
	{

		ProtocolHead head;
		(*ptr) >> head; 

		std::string content;
		(*ptr) >> content;

		std::cout << content << std::endl;
	}

	 void HandleEcho(ByteBuffer* ptr)
	 {
		 ProtocolHead temp_protocol_head;

		 (*ptr) >> temp_protocol_head;
		 uint32_t temp_len;
		 (*ptr) >> temp_len;
		 std::string send_data;
		 (*ptr) >> send_data;
		 std::cout << "ZmqEngine recv: " << send_data << std::endl;

		 //ByteBuffer buffer;

		 //uint32_t packet_len = 14;
		 //std::string data = "12345678";
		 //buffer << packet_len;
		 //buffer << data;

		 //temp_protocol_head.identification_++;
		 //SINGLETON_CTX->SendBuf(temp_protocol_head.cur_session_serial_num_, buffer.RdPos(), buffer.Length(), temp_protocol_head);

		 //std::stringstream ss;
		 //ss << "INSERT INTO `tb_role_farm` VALUES (NULL, '100004290', '1', '1', '1', 'qwert123');";
		 //std::string table;
		 //std::string query_statement = ss.str();
		 //CommandBaseEx* callback_cmd = NULL;
		 //ResultSet* query_result = NULL;


		 //SINGLETON_CTX->MysqlRPCClient()->SendRPCNotify(query_statement.c_str());

		 //ss.str("");
		 //ss << "INSERT INTO `tb_role_farm` VALUES (NULL, '2', '2', '2', '2', '2');";
		 //query_statement = ss.str();
		 //SINGLETON_CTX->MysqlRPCClient()->SendRPCNotifyLog(query_statement.c_str());
	 }

	 void HandleColseSession(ByteBuffer* ptr)
	 {
		 ProtocolHead temp_protocol_head;
		 (*ptr) >> temp_protocol_head;

		 std::cout << "ColseSession" << std::endl;
	 }

	  void HandleRoutingSession(ByteBuffer* ptr)
	  {

		  ProtocolHead temp_protocol_head;

		  (*ptr) >> temp_protocol_head;
		  uint32_t temp_len;
		  (*ptr) >> temp_len;
		  std::string send_data;
		  (*ptr) >> send_data;

		  std::cout << "HandleRoutingSession" << std::endl;
	  }
};

class UnitTestNetClient
{
public:
	UnitTestNetClient(void);
	~UnitTestNetClient(void);

public:
	int Run(void);

};


#endif
