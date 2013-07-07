#ifndef __SESSION_MANAGER_SESSION_MANAGER_H__
#define __SESSION_MANAGER_SESSION_MANAGER_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#include <string>
#include <vector>
#include <map>

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
#include "Utils/Singleton.h"
#include "Game/NetNodeId.h"
#include "Game/ServiceType.h"
#include "Net/Ctx.h"
#include "Utils/MD5.h"

#include "../LoginApp/LoginApp.h"
#include "../PlayerManager/PlayerManager.h"

class SessionManager : public IFacade
{
public:
	SessionManager(void);
	~SessionManager(void);

public:
	virtual void Initialize(void);
	virtual void Update(void);

public:
	enum
	{
		ERROR_CODE_SUCCESS = 0,                  //成功
		ERROR_CODE_ACCOUNT_PASSWORD_ERROR = 1,   //账号验证错误
		ERROR_CODE_UNVERIFIED = 2,               //未验证的连接操作

	};

	std::map<uint32_t, IDelegate*> GenOpcodes(void)
	{
		std::map<uint32_t, IDelegate*> temp;

		::RegisterOpcodes(temp, Opcodes::ECHO, this, &SessionManager::HandleEcho);

		::RegisterOpcodes(temp, Opcodes::CREATE_SESSION, this, &SessionManager::HandleCreate);
		::RegisterOpcodes(temp, Opcodes::CLOSE_SESSION, this, &SessionManager::HandleClose);
		::RegisterOpcodes(temp, Opcodes::REGISTER_GATEWAY_ADDRESS, this, &SessionManager::HandleRegisterGatewayAddress);

		::RegisterOpcodes(temp, Opcodes::PLATFORM_ACCOUNT_LOGIN_REQUEST, this, &SessionManager::HandlePlatformAccountLoginRequest);
		::RegisterOpcodes(temp, Opcodes::C_GET_ROLE_REQUEST, this, &SessionManager::HandleGetRoleRequest);
		::RegisterOpcodes(temp, Opcodes::G_PLAYER_BINDING_GATEWAY_RESPONSE, this, &SessionManager::HandlePlayerBindingGatewayResponse);
		
		return temp;
	}

public:

	void HandlePlatformAccountLoginRequest(ByteBuffer* ptr);
	void MySQLRPCAuthenticateAccount(ByteBuffer& ref_data, ResultSet* query_result);

	void HandleGetRoleRequest(ByteBuffer* ptr);
	void MySQLRPCGetRoleRequest(ByteBuffer& ref_data, ResultSet* query_result);

	void HandlePlayerBindingGatewayResponse(ByteBuffer* ptr);

	void HandleRegisterGatewayAddress(ByteBuffer* ptr);

	void HandleEcho(ByteBuffer* ptr);

	void HandleCreate(ByteBuffer* ptr);
	void HandleClose(ByteBuffer* ptr);
};

#endif