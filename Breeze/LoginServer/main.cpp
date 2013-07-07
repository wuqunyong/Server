#include <iostream>
#include <cstdlib>
#include <string>

#include "Net/Fd.h"
#include "Net/InitSock.h"

#include "LoginApp/LoginApp.h"

InitSock init_sock;

int main (int argc, char *argv [])
{
	std::cout << "login server hello world !" << std::endl;

	SINGLETON_LOGIN_APP->Init();
	SINGLETON_LOGIN_APP->Start();
	SINGLETON_LOGIN_APP->Stop();

	std::cout << "enter any key to end!" << std::endl;
	std::string end;
	std::cin >> end;

	return 0;
}
