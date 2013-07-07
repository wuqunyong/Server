#ifndef __GATEWAYAPP_GATEWAYAPP_H__
#define __GATEWAYAPP_GATEWAYAPP_H__

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

class GatewayApp
{
public:
	GatewayApp(void);
	~GatewayApp(void);

public:
	void Init(std::string service_id = "1", std::string service_name = "GatewayServer", std::string file = "../Shared/Config/Game.json");

	void Start(void);
	void Stop(void);

	FREEZE_NET::Ctx& GetCtx();

	bool SendTo(NetNodeId node, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad);

public:
	bool LoadConfig(std::string file);

	void AddSession(NetNodeId node, uint32_t serial_num);
	void RemoveSession(uint32_t serial_num);

	uint32_t FindSessionSerial(NetNodeId node);
	NetNodeId FindSessionNode(uint32_t serial_num);
	std::map<uint32_t, std::set<NetNodeId> >& GetInternalService(void);
	NetNodeId GetNodeId(void);

	void InsertInternalService(NetNodeId node);
	void DeleteInternalService(NetNodeId node);

	NetNodeAddress GetNetNodeAddress(void);

private:
	std::string service_name_;
	std::string service_id_;   //identity 唯一标识

	NetNodeId node_id_;

	FREEZE_NET::Ctx ctx_;

	std::string ip_;
	u_short port_;
	MySQLConnectionInfo rpc_server_conn_info_;
	MySQLConnectionInfo rpc_log_conn_info_;

	std::pair<std::string, u_short> login_addr_;

	//于网关直接相连的节点
	std::map<NetNodeId, uint32_t> node_session_map_;
	std::map<uint32_t, NetNodeId> session_node_map_;

	//内部注册的服务
	//key: 服务类型 例如：SERVICE_ROLE
	std::map<uint32_t, std::set<NetNodeId> > internal_service_;
};

#define SINGLETON_GATEWAY_APP Singleton<GatewayApp>::Instance()

#endif