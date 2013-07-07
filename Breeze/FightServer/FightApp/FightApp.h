#ifndef __FIGHTAPP_FIGHTAPP_H__
#define __FIGHTAPP_FIGHTAPP_H__

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

class FightApp
{
public:
	FightApp(void);
	~FightApp(void);

public:
	void Init(std::string service_id = "1", std::string service_name = "FightServer", std::string file = "../Shared/Config/Game.json");

	void Start(void);
	void Stop(void);

	FREEZE_NET::Ctx& GetCtx();

	bool SendTo(NetNodeId node, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad);
	bool SendToClient(uint32_t role_id, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad);
	bool SendToRoleServer(unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad);

	bool LoadConfig(std::string file);

public:
	void AddSession(NetNodeId node, uint32_t serial_num);
	void RemoveSession(uint32_t serial_num);
	uint32_t FindSessionSerial(NetNodeId node);
	NetNodeId FindSessionNode(uint32_t serial_num);

	std::map<NetNodeId, NetNodeId>& GetClientLinkingGateway(void);
	std::map<uint8_t, std::set<NetNodeId> >& GetGatewayLinkingService(void);
	void InsertGatewayLinkingService(NetNodeId node);

	std::map<uint32_t, uint32_t>& GetSessionRoleMap(void);
	std::set<uint32_t>& GetOnlineRoleSet(void);

private:
	std::string service_name_;
	std::string service_id_;   //identity 唯一标识

	NetNodeId node_id_;

	FREEZE_NET::Ctx ctx_;

	std::string ip_;
	u_short port_;
	MySQLConnectionInfo rpc_server_conn_info_;
	MySQLConnectionInfo rpc_log_conn_info_;

	std::map<uint32_t, std::pair<std::string, u_short> > gateway_addr_map_;

	std::map<NetNodeId, uint32_t> node_session_map_;
	std::map<uint32_t, NetNodeId> session_node_map_;

	std::map<NetNodeId, NetNodeId> client_linking_gateway_;
	std::map<uint8_t, std::set<NetNodeId> > gateway_linking_serveice_;

	//路由通信SEESION与ROLE_ID的映射表 即：head.suspend_session_serial_num_ 与 role_id的映射
	//key: head.suspend_session_serial_num_
	//value : role_id
	std::map<uint32_t, uint32_t> session_role_map_;

	//在线ROLE集合 role_id
	std::set<uint32_t> online_role_set_;
};

#define SINGLETON_FIGHT_APP Singleton<FightApp>::Instance()

#endif