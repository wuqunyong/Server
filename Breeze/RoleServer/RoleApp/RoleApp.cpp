#include "RoleApp.h"

#include <fstream>

#include "../SessionManager/SessionManager.h"
#include "Game/ServiceType.h"

#include <json/json.h>
using namespace Json; 

RoleApp::RoleApp(void)
	: ctx_(8)
{

}

RoleApp::~RoleApp(void)
{

}

void RoleApp::Init(std::string service_name, std::string service_id, std::string file)
{
	this->service_name_ = service_name;
	this->service_id_ = service_id;

	this->node_id_.service_ = SERVICE_ROLE;
	this->node_id_.id_ = ::atoi(service_id.c_str());

	this->LoadConfig(file);
}

void RoleApp::Start(void)
{
	IFacade* ptr = new SessionManager();
	this->ctx_.GetOpcodes().Plug(ptr, ptr->GenOpcodes());
	this->ctx_.GetSocketBase()->Bind(this->port_, this->ip_.c_str());

	this->ctx_.MysqlRPCServer()->Connect(this->rpc_server_conn_info_);
	this->ctx_.MysqlRPCLog()->Connect(this->rpc_log_conn_info_);


	for (std::map<uint32_t, std::pair<std::string, u_short> >::iterator ite = gateway_addr_map_.begin();
		ite != gateway_addr_map_.end();
		ite++)
	{
		//连接到GatewayServer
		uint32_t serial_num = this->ctx_.GetSocketBase()->Connect(ite->second.second, ite->second.first.c_str());

		//在RoleServer注册 SERVICE_GATEWAY 网关服务
		NetNodeId node(SERVICE_GATEWAY, ite->first);
		this->AddSession(node,serial_num);

		//发送CREATE_SESSION注册包
		ByteBuffer buffer;
		buffer << this->node_id_.service_;
		buffer << this->node_id_.id_;

		std::string validate = "RoleServer";
		buffer << validate;

		ProtocolHead protocol_head;
		protocol_head.opcode_ = Opcodes::CREATE_SESSION;
		this->ctx_.SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);
	}
}

void RoleApp::Stop(void)
{
	std::cout << "enter quit to quit" << std::endl;
	std::string val;
	std::cin >> val;

	if (val == "quit")
	{
		//this->ctx_.Terminate();
	}
	else
	{
		std::cout << "enter quit to quit" << std::endl;
	}
	
}

FREEZE_NET::Ctx& RoleApp::GetCtx()
{
	return this->ctx_;
}

bool RoleApp::LoadConfig(std::string file)
{
	std::ifstream in(file.c_str());    
	std::string str;   
	std::string msg;    
	if(in.is_open())   
	{      
		while(in>>str)           
			msg += str;   
	}     
	in.close();


	Reader reader;
	Value value;


	if (reader.parse(msg, value))
	{
		const Json::Value gateway_src = value["GatewayServer"];

		for (unsigned int i=0; i<10; i++)
		{

			std::stringstream ss;
			ss << i;
			std::string index = ss.str();

			bool result = gateway_src[index].empty();
			if (!result)
			{
				std::pair<std::string, u_short> gateway_addr;
				gateway_addr.first = gateway_src[index]["ip"].asString();
				gateway_addr.second = gateway_src[index]["port"].asUInt();

				gateway_addr_map_[i] = gateway_addr;
			}
		}

		const Json::Value src = value[this->service_name_];

		 ip_ = src["ip"].asString();
		 port_ = src["port"].asUInt();

		 const Json::Value rpc_server_db = value[this->service_name_]["db"]["rpc_server"];
		 rpc_server_conn_info_.host = rpc_server_db["host"].asString();
		 rpc_server_conn_info_.user = rpc_server_db["user"].asString();
		 rpc_server_conn_info_.password = rpc_server_db["password"].asString();
		 rpc_server_conn_info_.database = rpc_server_db["database"].asString();
		 rpc_server_conn_info_.port = 3306;


		 const Json::Value rpc_log_db = value[this->service_name_]["db"]["rpc_server"];
		 rpc_log_conn_info_.host = rpc_log_db["host"].asString();
		 rpc_log_conn_info_.user = rpc_log_db["user"].asString();
		 rpc_log_conn_info_.password = rpc_log_db["password"].asString();
		 rpc_log_conn_info_.database = rpc_log_db["database"].asString();
		 rpc_log_conn_info_.port = 3306;
	}

	return true;
}


void RoleApp::AddSession(NetNodeId node, uint32_t serial_num)
{
	std::map<NetNodeId, uint32_t>::iterator ite_node = node_session_map_.find(node);
	if (ite_node == node_session_map_.end())
	{
		node_session_map_[node] = serial_num;
	}
	else
	{
		uint32_t previous_serial_num = ite_node->second;

		RemoveSession(previous_serial_num);

		node_session_map_[node] = serial_num;
	}
	

	std::map<uint32_t, NetNodeId>::iterator ite_serial = session_node_map_.find(serial_num);
	if (ite_serial == session_node_map_.end())
	{
		session_node_map_[serial_num] = node;
	}
	else
	{
		std::cout << "repeat serial_num error !" << __FILE__ << " " << __LINE__ << std::endl;
	}
}

void RoleApp::RemoveSession(uint32_t serial_num)
{
	std::map<uint32_t, NetNodeId>::iterator ite_serial = session_node_map_.find(serial_num);
	if (ite_serial != session_node_map_.end())
	{
		//客户端重复注册，提掉之前注册的连接下线
		uint8_t temp_service = ite_serial->second.service_;

		std::map<NetNodeId, uint32_t>::iterator ite_node = node_session_map_.find(ite_serial->second);
		if (ite_node != node_session_map_.end())
		{
			node_session_map_.erase(ite_node);
		}

		session_node_map_.erase(ite_serial);

		if (temp_service == SERVICE_CLIENT)
		{
			this->ctx_.Close(serial_num);
		}
	}
}

uint32_t RoleApp::FindSessionSerial(NetNodeId node)
{
	std::map<NetNodeId, uint32_t>::iterator ite_node = node_session_map_.find(node);
	if (ite_node == node_session_map_.end())
	{
		return 0;
	}
	else
	{
		return ite_node->second;
	}
}

std::map<NetNodeId, NetNodeId>& RoleApp::GetClientLinkingGateway(void)
{
	return client_linking_gateway_;
}

std::map<uint8_t, std::set<NetNodeId> >& RoleApp::GetGatewayLinkingService(void)
{
	return this->gateway_linking_serveice_;
}

void RoleApp::InsertGatewayLinkingService(NetNodeId node)
{
	if (node.service_ == SERVICE_ROLE || node.service_ == SERVICE_FIGHT)
	{
		std::cout << "receive gateway linking service node: " << (uint32_t)node.service_ << " " << node.id_ << std::endl;

		std::map<uint8_t, std::set<NetNodeId> >::iterator ite = this->gateway_linking_serveice_.find(node.service_);
		if (ite == this->gateway_linking_serveice_.end())
		{
			std::set<NetNodeId> node_set;
			node_set.insert(node);

			this->gateway_linking_serveice_[node.service_] = node_set;
		}
		else
		{
			ite->second.insert(node);
		}
	}
}

NetNodeId RoleApp::GetNetNodeId(void)
{
	return this->node_id_;
}

std::map<uint32_t, uint32_t>& RoleApp::GetSessionRoleMap(void)
{
	return this->session_role_map_;
}

std::set<uint32_t>& RoleApp::GetOnlineRoleSet(void)
{
	return this->online_role_set_;
}

std::map<uint32_t, NetNodeId>& RoleApp::GetSierialNumServiceMap(void)
{
	return this->serial_num_gateway_linking_service_map_;
}

bool RoleApp::SendTo(NetNodeId node, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad)
{
	std::map<NetNodeId, uint32_t>::iterator ite_node = node_session_map_.find(node);
	if (ite_node == node_session_map_.end())
	{
		return false;
	}
	else
	{
		uint32_t serial_num = ite_node->second;
		return this->ctx_.SendBuf(serial_num, ptr, ptr_size, protocol_haad);
	}
}

bool RoleApp::SendToClient(uint32_t role_id, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad)
{
	NetNodeId client_node;
	client_node.service_ = SERVICE_CLIENT;
	client_node.id_ = role_id;

	std::map<NetNodeId, NetNodeId>::iterator ite = this->client_linking_gateway_.find(client_node);
	if (ite == this->client_linking_gateway_.end())
	{
		return false;
	}
	else
	{
		std::map<NetNodeId, uint32_t>::iterator ite_node = node_session_map_.find(ite->second);
		if (ite_node == node_session_map_.end())
		{
			return false;
		}
		else
		{
			uint32_t serial_num = ite_node->second;
			return this->ctx_.SendBuf(serial_num, ptr, ptr_size, protocol_haad);
		}
	}
}

bool RoleApp::SendToFightServer(unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad)
{
	//查找与之编号相对应的网关节点，没有就第一个可用节点
	NetNodeId route_node;
	route_node.service_ = SERVICE_GATEWAY;
	route_node.id_ = this->node_id_.id_;

	//源，目的节点地址：
	protocol_haad.origin_service_ = this->node_id_.service_;
	protocol_haad.origin_session_serial_num_ = this->node_id_.id_;

	protocol_haad.destination_service_ = SERVICE_FIGHT;
	//protocol_haad.destination_session_serial_num_ = 1;

	std::map<NetNodeId, uint32_t>::iterator ite_node = node_session_map_.find(route_node);
	if (ite_node == node_session_map_.end())
	{
		for (std::map<NetNodeId, uint32_t>::iterator find_ite = node_session_map_.begin();
			find_ite != node_session_map_.end();
			find_ite++)
		{
			if (find_ite->first.service_ == SERVICE_GATEWAY)
			{
				uint32_t serial_num = find_ite->second;
				return this->ctx_.SendBuf(serial_num, ptr, ptr_size, protocol_haad);
			}
		}

		return false;
	}
	else
	{
		uint32_t serial_num = ite_node->second;
		return this->ctx_.SendBuf(serial_num, ptr, ptr_size, protocol_haad);
	}

}