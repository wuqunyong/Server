#ifndef __ROLEAPP_ROLEAPP_H__
#define __ROLEAPP_ROLEAPP_H__

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

class RoleApp
{
public:
	RoleApp(void);
	~RoleApp(void);

public:
	void Init(std::string service_name = "RoleServer", std::string service_id = "1", std::string file = "../Shared/Config/Game.json");

	void Start(void);
	void Stop(void);

	FREEZE_NET::Ctx& GetCtx();

	bool SendTo(NetNodeId node, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad);
	bool LoadConfig(std::string file);

	bool SendToClient(uint32_t role_id, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad);
	bool SendToFightServer(unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad);

public:
	void AddSession(NetNodeId node, uint32_t serial_num);
	void RemoveSession(uint32_t serial_num);
	uint32_t FindSessionSerial(NetNodeId node);
	std::map<NetNodeId, NetNodeId>& GetClientLinkingGateway(void);
	std::map<uint8_t, std::set<NetNodeId> >& GetGatewayLinkingService(void);

	void InsertGatewayLinkingService(NetNodeId node);

	NetNodeId GetNetNodeId(void);
	std::map<uint32_t, uint32_t>& GetSessionRoleMap(void);
	std::set<uint32_t>& GetOnlineRoleSet(void);

	std::map<uint32_t, NetNodeId>& GetSierialNumServiceMap(void);
private:
	std::string service_name_;
	std::string service_id_;   //identity 唯一标识

	NetNodeId node_id_;

	FREEZE_NET::Ctx ctx_;

	std::string ip_;
	u_short port_;
	MySQLConnectionInfo rpc_server_conn_info_;
	MySQLConnectionInfo rpc_log_conn_info_;

	//配置文件里的网关地址
	std::map<uint32_t, std::pair<std::string, u_short> > gateway_addr_map_;

	std::map<NetNodeId, uint32_t> node_session_map_;
	std::map<uint32_t, NetNodeId> session_node_map_;

	//最近客服端ROLE于那个网关相连，ROLE下线后还保存
	std::map<NetNodeId, NetNodeId> client_linking_gateway_;

	//网关连接的内部服务节点
	std::map<uint8_t, std::set<NetNodeId> > gateway_linking_serveice_;
	std::map<uint32_t, NetNodeId> serial_num_gateway_linking_service_map_;

	//路由通信SEESION与ROLE_ID的映射表 即：head.suspend_session_serial_num_ 与 role_id的映射
	//key: head.suspend_session_serial_num_
	//value : role_id
	std::map<uint32_t, uint32_t> session_role_map_;

	//在线ROLE集合 role_id
	std::set<uint32_t> online_role_set_;
};

#define SINGLETON_ROLE_APP Singleton<RoleApp>::Instance()

#endif