#include <iostream>
#include <cstdlib>
#include <string>

#include "Net/Fd.h"
#include "Net/InitSock.h"

#include "RoleApp/RoleApp.h"

InitSock init_sock;

int main (int argc, char *argv [])
{
	std::cout << "role server hello world !" << std::endl;

	SINGLETON_ROLE_APP->Init();
	SINGLETON_ROLE_APP->Start();
	SINGLETON_ROLE_APP->Stop();

	std::cout << "enter any key to end!" << std::endl;
	std::string end;
	std::cin >> end;

	return 0;
}
