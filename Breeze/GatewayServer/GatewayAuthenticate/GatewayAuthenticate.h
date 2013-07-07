#ifndef __GATEWAY_AUTHENTICATE_GATEWAY_AUTHENTICATE_H__
#define __GATEWAY_AUTHENTICATE_GATEWAY_AUTHENTICATE_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>
#include <set>

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

#include "Net/Fd.h"

class GatewayAuthenticate
{
public:
	GatewayAuthenticate(void);
	~GatewayAuthenticate(void);

	std::map<std::string, std::set<uint32_t> >& GetKeyRoleSet(void);

private:
	std::map<std::string, std::set<uint32_t> > key_role_set_;
};

#define SINGLETON_GATEWAY_AUTHENTICATE Singleton<GatewayAuthenticate>::Instance()

#endif