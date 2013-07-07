#include "GatewayApp.h"

#include <fstream>

#include "../SessionManager/SessionManager.h"
#include "Game/ServiceType.h"

#include <json/json.h>
using namespace Json; 

GatewayApp::GatewayApp(void)
{

}

GatewayApp::~GatewayApp(void)
{

}

void GatewayApp::Init(std::string service_id, std::string service_name, std::string file)
{
	this->service_name_ = service_name;
	this->service_id_ = service_id;

	this->node_id_.service_ = SERVICE_GATEWAY;
	this->node_id_.id_ = ::atoi(service_id.c_str());

	this->LoadConfig(file);
}

void GatewayApp::Start(void)
{
	IFacade* ptr = new SessionManager();
	this->ctx_.GetOpcodes().Plug(ptr, ptr->GenOpcodes());
	this->ctx_.GetSocketBase()->Bind(this->port_, this->ip_.c_str());

	std::cout << this->ip_ << " " << this->port_ << std::endl;

	this->ctx_.MysqlRPCServer()->Connect(this->rpc_server_conn_info_);
	this->ctx_.MysqlRPCLog()->Connect(this->rpc_log_conn_info_);


	//连接到LoginServer
	uint32_t serial_num = this->ctx_.GetSocketBase()->Connect(login_addr_.second, login_addr_.first.c_str());

	//在GatewayServer注册 SERVICE_LOGIN 服务
	NetNodeId node(SERVICE_LOGIN, 1);
	this->AddSession(node,serial_num);

	//发送CREATE_SESSION注册包
	ByteBuffer buffer;
	buffer << node_id_.service_;
	buffer << node_id_.id_;

	std::string validate = "GatewayServer";
	buffer << validate;

	ProtocolHead protocol_head;
	protocol_head.opcode_ = Opcodes::CREATE_SESSION;
	this->ctx_.SendBuf(serial_num, buffer.RdPos(), buffer.Length(), protocol_head);

	//向LOGIN_SERVER注册GATEWAY_SERVER地址
	ByteBuffer register_buffer;

	NetNodeId gateway_node = this->node_id_;

	NetNodeAddress gateway_address;
	gateway_address.ip_ = this->ip_;
	gateway_address.port_ = this->port_;

	register_buffer << gateway_node;
	register_buffer << gateway_address;

	ProtocolHead register_head;
	register_head.opcode_ = Opcodes::REGISTER_GATEWAY_ADDRESS;
	this->ctx_.SendBuf(serial_num, register_buffer.RdPos(), register_buffer.Length(), register_head);

}

void GatewayApp::Stop(void)
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

FREEZE_NET::Ctx& GatewayApp::GetCtx()
{
	return this->ctx_;
}

bool GatewayApp::LoadConfig(std::string file)
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
		const Json::Value login_src = value["LoginServer"];
		login_addr_.first = login_src["ip"].asString();
		login_addr_.second = login_src["port"].asUInt();


		const Json::Value src = value[this->service_name_][this->service_id_];

		 ip_ = src["ip"].asString();
		 port_ = src["port"].asUInt();

		 const Json::Value rpc_server_db = value[this->service_name_][this->service_id_]["db"]["rpc_server"];
		 rpc_server_conn_info_.host = rpc_server_db["host"].asString();
		 rpc_server_conn_info_.user = rpc_server_db["user"].asString();
		 rpc_server_conn_info_.password = rpc_server_db["password"].asString();
		 rpc_server_conn_info_.database = rpc_server_db["database"].asString();
		 rpc_server_conn_info_.port = 3306;


		 const Json::Value rpc_log_db = value[this->service_name_][this->service_id_]["db"]["rpc_server"];
		 rpc_log_conn_info_.host = rpc_log_db["host"].asString();
		 rpc_log_conn_info_.user = rpc_log_db["user"].asString();
		 rpc_log_conn_info_.password = rpc_log_db["password"].asString();
		 rpc_log_conn_info_.database = rpc_log_db["database"].asString();
		 rpc_log_conn_info_.port = 3306;
	}

	return true;
}


void GatewayApp::AddSession(NetNodeId node, uint32_t serial_num)
{
	std::map<NetNodeId, uint32_t>::iterator ite_node = node_session_map_.find(node);
	if (ite_node == node_session_map_.end())
	{
		node_session_map_[node] = serial_num;
	}
	else
	{
		if (node.service_ != SERVICE_CLIENT)
		{
			std::cout << "AddSession repeat internal : " << node.service_ << " " << __FILE__ << " " << __LINE__ << std::endl; 
		}
		else
		{
			//客户端重复登录，只会断掉之前连接的客户端，
			//服务器内部虽然会在GATEWAY_SERVER断开连接时发送CLOSE_SESSION数据包，但，session_node_map_中相关的节点关系已清除
			//ROLE_SERVER收不到CLOSE_SESSION数据包
			std::cout << "AddSession repeat client : " << node.service_ << " " << __FILE__ << " " << __LINE__ << std::endl; 

			uint32_t previous_serial_num = ite_node->second;

			RemoveSession(previous_serial_num);

			node_session_map_[node] = serial_num;
		}
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

void GatewayApp::InsertInternalService(NetNodeId node)
{
	if (node.service_ == SERVICE_ROLE || node.service_ == SERVICE_FIGHT)
	{
		std::cout << "insert internal service node: " << (uint32_t)node.service_ << " " << node.id_ << std::endl;

		std::map<uint32_t, std::set<NetNodeId> >::iterator ite = internal_service_.find(node.service_);
		if (ite == internal_service_.end())
		{
			std::set<NetNodeId> node_set;
			node_set.insert(node);

			internal_service_[node.service_] = node_set;
		}
		else
		{
			ite->second.insert(node);
		}
	}
}

void GatewayApp::DeleteInternalService(NetNodeId node)
{
	if (node.service_ == SERVICE_ROLE || node.service_ == SERVICE_FIGHT)
	{
		std::cout << "delete internal service node: " << (uint32_t)node.service_ << " " << node.id_ << std::endl;

		std::map<uint32_t, std::set<NetNodeId> >::iterator ite = internal_service_.find(node.service_);
		if (ite != internal_service_.end())
		{
			std::set<NetNodeId>::iterator node_ite = ite->second.find(node);
			if (node_ite != ite->second.end())
			{		
				ite->second.erase(node_ite);			
				std::cout << "deleted node: " << (uint32_t)node.service_ << " " << node.id_ << std::endl;
			}
		}

	}
}

void GatewayApp::RemoveSession(uint32_t serial_num)
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

uint32_t GatewayApp::FindSessionSerial(NetNodeId node)
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

NetNodeId GatewayApp::FindSessionNode(uint32_t serial_num)
{
	std::map<uint32_t, NetNodeId>::iterator ite = session_node_map_.find(serial_num);
	if (ite == session_node_map_.end())
	{
		NetNodeId empty;
		return empty;
	}
	else
	{
		return ite->second;
	}
}

std::map<uint32_t, std::set<NetNodeId> >& GatewayApp::GetInternalService(void)
{
	return this->internal_service_;
}

NetNodeId GatewayApp::GetNodeId(void)
{
	return this->node_id_;
}

bool GatewayApp::SendTo(NetNodeId node, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad)
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

NetNodeAddress GatewayApp::GetNetNodeAddress(void)
{
	NetNodeAddress node;
	node.ip_ = this->ip_;
	node.port_ = this->port_;

	return node;
}