#include "NetServer.h"
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

UnitTestNetServer::UnitTestNetServer(void)
{

}

UnitTestNetServer::~UnitTestNetServer(void)
{

}

int UnitTestNetServer::Run(void)
{
	//FREEZE_NET::Ctx context(2);
	IFacade* ptr = new ServerFacade();
	SINGLETON_CTX->GetOpcodes().Plug(ptr, ptr->GenOpcodes());

	SINGLETON_CTX->GetSocketBase()->Bind(5099, "127.0.0.1");

	std::cout << "Enter any key to close !" << std::endl;
	std::string end;
	std::cin >> end;



	std::cout << "Enter any key to eixt !" << std::endl;
	std::string end_1;
	std::cin >> end_1;
	return 0;
}

