#include <iostream>
#include <cstdlib>
#include <string>

#include "Net/Fd.h"

#include "MySQLDb/MySQLDb.h"
#include "JsonCpp/JsonCpp.h"
#include "Serialization/Serialization.h"
#include "RPCMySQL/RPCMySQL.h"
#include "NetServer/NetServer.h"
#include "NetClient/NetClient.h"

#include "Net/InitSock.h"

InitSock init_sock;

int main (int argc, char *argv [])
{
	std::cout << "hello world !" << std::endl;


	//UnitTestMySQL unit_test;
	//unit_test.Run();

	//UnitTestJsonCpp unit_test;
	//unit_test.Run();

	//UnitTestSerialization unit_test;
	//unit_test.Run();

	//UnitTestRPCMysql unit_test;
	//unit_test.Run();

	//UnitTestNetServer unit_test;
	//unit_test.Run();

	UnitTestNetClient unit_test;
	unit_test.Run();

	std::cout << "enter any key to end!" << std::endl;
	std::string end;
	std::cin >> end;

	return 0;
}
