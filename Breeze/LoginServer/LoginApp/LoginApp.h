#ifndef __LOGINAPP_LOGINAPP_H__
#define __LOGINAPP_LOGINAPP_H__

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

class LoginApp
{
public:
	LoginApp(void);
	~LoginApp(void);

public:
	void Init(std::string service_name = "LoginServer", std::string service_id = "1", std::string file = "../Shared/Config/Game.json");

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

	void InsertLinkingGateway(NetNodeId node);
	void DeleteLinkingGateway(NetNodeId node);

	std::set<NetNodeId>& GetLinkingGateway(void);
	std::map<NetNodeId, NetNodeAddress>& GetGatewayAddressMap(void);

	bool ChooseGatewayAddress(uint32_t player_id, NetNodeAddress& address, NetNodeId& gateway_node);

	std::map<uint32_t, NetNodeId>& GetAccountGatewayAddress(void);
	std::map<uint32_t, uint32_t>& GetVerifiedAccount();
	std::map<uint32_t, uint32_t>& GetGetVerifiedPlayerSerial(void);

public:
	void ResponseRequestError(uint32_t serial_num, uint32_t request_opcodes);

private:
	std::string service_name_;
	std::string service_id_;   //identity 唯一标识

	NetNodeId node_id_;

	FREEZE_NET::Ctx ctx_;

	std::string ip_;
	u_short port_;
	MySQLConnectionInfo rpc_server_conn_info_;
	MySQLConnectionInfo rpc_log_conn_info_;


	std::map<NetNodeId, uint32_t> node_session_map_;
	std::map<uint32_t, NetNodeId> session_node_map_;

	//连接的网关服务
	std::set<NetNodeId> linking_gateway_set_;
	std::map<NetNodeId, NetNodeAddress> gateway_addr_map_;

	//平台账号登陆绑定GATEWAY_SERVER
	//KEY:平台账号名 player_id
	//VALUE:角色连接的网关地址
	std::map<uint32_t, NetNodeId> platform_account_binding_gateway_;

	//key: serial_num
	//value: player_id
	std::map<uint32_t, uint32_t> verified_account_;

	//key: player_id
	//value: serial_num
	std::map<uint32_t, uint32_t> verified_player_serial_;
};

#define SINGLETON_LOGIN_APP Singleton<LoginApp>::Instance()

#endif