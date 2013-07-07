#ifndef __SHARED_NET_INIT_SOCK_H__
#define __SHARED_NET_INIT_SOCK_H__

#ifdef WIN32

#include <assert.h>

#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "WS2_32")	//Á´½Óµ½WS2_32.lib

class InitSock		
{
public:
	InitSock(void)
	{
		WORD version_requested = MAKEWORD(2, 2);
		WSADATA wsa_data;
		int rc = WSAStartup(version_requested, &wsa_data);
		assert(rc == 0);
		assert(LOBYTE(wsa_data.wVersion) == 2 && HIBYTE(wsa_data.wVersion) == 2);
	}
	~InitSock(void)
	{	
		int rc = WSACleanup();
		assert(rc != SOCKET_ERROR);
	}
};
#else
class InitSock		
{
public:
	InitSock(void)
	{
	}
	~InitSock(void)
	{	
	}
};
#endif

#endif