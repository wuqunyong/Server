#include "NetClient.h"
#include <string>
#include <sstream>

#include "Net/Fd.h"
#include "Utils/CommandBase.h"

#include "Db/ResultSet.h"
#include "Net/Ctx.h"
#include "Net/MySQLRPCClient.h"
#include "Net/MySQLRPCServer.h"
#include "Net/IOThread.h"
#include "Net/SocketBase.h"

#include "Game/ServiceType.h"

#include "Serialization/ByteBuffer.h"
#include "Utils/MD5.h"
#include "Utils/Character.h"

#ifndef WIN32
#include <unistd.h>
#endif


UnitTestNetClient::UnitTestNetClient(void)
{

}

UnitTestNetClient::~UnitTestNetClient(void)
{

}

int UnitTestNetClient::Run(void)
{
	//FREEZE_NET::Ctx context(2);

	std::cout << "md5 of 'grape': " << Md5String("grape") << std::endl;


	std::string player_name = "ÄãºÃ";
	char* utf8 = SINGLETON_CHARACTER_CONVERTER->ConvertToUtf8((char*)player_name.c_str());
	player_name = utf8;
	free(utf8);

	std::cout << player_name << std::endl;

	char * ascii_player_name = SINGLETON_CHARACTER_CONVERTER->ConvertFromUtf8((char*)player_name.c_str());
	player_name = ascii_player_name;
	free(ascii_player_name);

	std::cout << player_name << std::endl;

	IFacade* ptr = new TestFacade();
	SINGLETON_CTX->GetOpcodes().Plug(ptr, ptr->GenOpcodes());
	//SINGLETON_CTX->GetSocketBase()->Bind(9201, "127.0.0.1");

	MySQLConnectionInfo conn_info;
	conn_info.host = "127.0.0.1";
	conn_info.user = "root";
	conn_info.password = "123456";
	conn_info.database = "test";
	conn_info.port = 3306;
	SINGLETON_CTX->MysqlRPCServer()->Connect(conn_info);
	SINGLETON_CTX->MysqlRPCLog()->Connect(conn_info);

	//FREEZE_NET::IPollEvents* ptr_timer =  new TestTimer();
	//SINGLETON_CTX->MysqlRPCClient()->GetPoller()->AddTimer(5000, ptr_timer, 5);
	//SINGLETON_CTX->MysqlRPCClient()->GetPoller()->AddTimer(15000, ptr_timer, 15);
	//SINGLETON_CTX->MysqlRPCClient()->GetPoller()->AddTimer(25000, ptr_timer, 25);

	int loops = 100;
	while(loops--)
	{

		{
			//windows login
			uint32_t serial_num = SINGLETON_CTX->GetSocketBase()->Connect(9101, "192.168.4.41");//windows login


			ByteBuffer register_buffer;

			ProtocolHead register_head;
			register_head.opcode_ = Opcodes::PLATFORM_ACCOUNT_LOGIN_REQUEST;

			std::cout << "input account_name : " << std::endl;
			std::string account_name;
			std::cin >> account_name;

			std::cout << "input passwd : " << std::endl;
			std::string passwd;
			std::cin >> passwd;

			//account_name = "test";
			//passwd = "test";

			register_buffer << account_name;
			register_buffer << passwd;

			SINGLETON_CTX->SendBuf(serial_num, register_buffer.RdPos(), register_buffer.Length(), register_head);



			std::cout << "Enter any key to eixt !" << std::endl;
			std::string end_2;
			std::cin >> end_2;

#ifdef WIN32
			Sleep(500);
#else
			usleep((500)*1000);
#endif

			SINGLETON_CTX->Close(serial_num);


		}
		/*
		uint32_t serial_num = SINGLETON_CTX->GetSocketBase()->Connect(9201, "192.168.4.41");//windows
		//uint32_t serial_num = SINGLETON_CTX->GetSocketBase()->Connect(9201, "192.168.4.54"); //linux
		ByteBuffer register_buffer;

		ProtocolHead register_head;
		register_head.opcode_ = Opcodes::CREATE_SESSION;

		uint8_t service = SERVICE_CLIENT;
		uint32_t id = serial_num;
		register_buffer << service;
		register_buffer << id;

		std::string validate = "ClientServer";
		register_buffer << validate;

		SINGLETON_CTX->SendBuf(serial_num, register_buffer.RdPos(), register_buffer.Length(), register_head);


		ByteBuffer buffer;

		std::string data = "request";
	
		std::stringstream ss;
		ss << loops;

		std::string index = ss.str();
		data += index;

		buffer << data;


		ProtocolHead protocol_head;
		protocol_head.identification_ = 123;
		protocol_head.opcode_ = Opcodes::REQUEST_TEST;
		//protocol_head.origin_service_ = SERVICE_CLIENT;
		//protocol_head.origin_session_serial_num_ = serial_num;

		protocol_head.origin_service_ = 0;
		protocol_head.origin_session_serial_num_ = 0;

		SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);

		//ByteBuffer buffer;

		//uint32_t packet_len = 14;
		//std::string data = "12345678";
		//buffer << packet_len;
		//buffer << data;

		//ProtocolHead protocol_head;
		//protocol_head.opcode_ = Opcodes::ECHO;
		//protocol_head.origin_session_serial_num_ = serial_num;
		//SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);

		//ByteBuffer routing_buffer;

		//ProtocolHead routing_head;
		//routing_head.opcode_ = Opcodes::ROUTING;
		//routing_head.origin_session_serial_num_ = serial_num;

		//protocol_head.total_length_ = sizeof(ProtocolHead) + buffer.Length();

		//routing_buffer << protocol_head;
		//routing_buffer << packet_len;
		//routing_buffer << data;
		//SINGLETON_CTX->SendBuf(serial_num, routing_buffer.RdPos(), routing_buffer.Length(), routing_head);

		//SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);
		//SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);
		//SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);
		//SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);

		//std::cout << "Enter any key to eixt !" << std::endl;
		//std::string end;
		//std::cin >> end;

		std::cout << "Enter any key to eixt !" << std::endl;
		std::string end_2;
		std::cin >> end_2;

#ifdef WIN32
		Sleep(500);
#else
		usleep((500)*1000);
#endif
		
		SINGLETON_CTX->Close(serial_num);

//#ifdef WIN32
//		Sleep(300000);
//#else
//		usleep((300000)*1000);
//#endif
*/
	}


	std::cout << "Enter any key to eixt !" << std::endl;
	std::string end_2;
	std::cin >> end_2;

	uint32_t serial_num = SINGLETON_CTX->GetSocketBase()->Connect(5099, "127.0.0.1");

	ByteBuffer buffer;

	uint32_t packet_len = 14;
	std::string data = "12345678";
	buffer << packet_len;
	buffer << data;

	ProtocolHead protocol_head;
	protocol_head.opcode_ = Opcodes::ECHO;

	SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);
	//SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);
	//SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);
	//SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);
	//SINGLETON_CTX->SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);

	//std::cout << "Enter any key to eixt !" << std::endl;
	//std::string end;
	//std::cin >> end;

#ifdef WIN32
	Sleep(500);
#else
	usleep((500)*1000);
#endif

	SINGLETON_CTX->Close(serial_num);

	std::cout << "Enter any key to eixt !" << std::endl;
	std::string end_1;
	std::cin >> end_1;
	return 0;
}

