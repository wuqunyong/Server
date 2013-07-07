#ifndef __LOGINAPP_PLAYERMANAGER_H__
#define __LOGINAPP_PLAYERMANAGER_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

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
#include "Net/Ctx.h"

struct RoleRegisterInfo
{
	uint32_t role_id_;
	std::string name_;
	uint32_t sex_;
	uint32_t profession_;

	bool operator==(const RoleRegisterInfo& rhs) const 
	{
		return this->role_id_ == rhs.role_id_;
	}
	bool operator< (const RoleRegisterInfo& rhs) const 
	{
		return  this->role_id_ < rhs.role_id_;
	}
};

extern ByteBuffer& operator >> (ByteBuffer& stream, RoleRegisterInfo& data);
extern ByteBuffer& operator << (ByteBuffer& stream, RoleRegisterInfo data);

class PlayerManager
{
public:
	PlayerManager(void);
	~PlayerManager(void);

public:
	void ChooseGatewayAddress(uint32_t player_id, std::set<uint32_t> role_set);

	std::map<uint32_t, std::set<RoleRegisterInfo> >& GetPlayerInfo(void);

public:
	void ResponseGetRole(uint32_t serial_num, uint32_t player_id, std::set<RoleRegisterInfo> role_set);

private:
	std::map<uint32_t, std::set<RoleRegisterInfo> > player_info_;
};

#define SINGLETON_LOGIN_PLAYER_MANAGER Singleton<PlayerManager>::Instance()

#endif