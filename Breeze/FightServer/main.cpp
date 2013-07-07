#include <iostream>
#include <cstdlib>
#include <string>

#include "Net/Fd.h"
#include "Net/InitSock.h"

#include "FightApp/FightApp.h"

InitSock init_sock;

int main (int argc, char *argv [])
{
	std::cout << "fight server hello world !" << std::endl;

	if (1== argc)
	{
		SINGLETON_FIGHT_APP->Init();
		SINGLETON_FIGHT_APP->Start();
		SINGLETON_FIGHT_APP->Stop();
	}
	else if (2 == argc)
	{
		std::string id = argv[1];
		SINGLETON_FIGHT_APP->Init(id);
		SINGLETON_FIGHT_APP->Start();
		SINGLETON_FIGHT_APP->Stop();
	}
	else
	{
		std::cout << "parameters count error !" << __FILE__ << " " << __LINE__ << std::endl;
	}

	std::cout << "enter any key to end!" << std::endl;
	std::string end;
	std::cin >> end;

	return 0;
}
