#include "LoginApp.h"

#include <time.h>

#include <fstream>

#include "../SessionManager/SessionManager.h"
#include "Game/ServiceType.h"

#include <json/json.h>
using namespace Json; 

LoginApp::LoginApp(void)
{
	::srand(time(NULL));
}

LoginApp::~LoginApp(void)
{

}

void LoginApp::Init(std::string service_name, std::string service_id, std::string file)
{
	this->service_name_ = service_name;
	this->service_id_ = service_id;

	this->node_id_.service_ = SERVICE_LOGIN;
	this->node_id_.id_ = ::atoi(service_id.c_str());

	this->LoadConfig(file);
}

void LoginApp::Start(void)
{
	IFacade* ptr = new SessionManager();
	this->ctx_.GetOpcodes().Plug(ptr, ptr->GenOpcodes());
	this->ctx_.GetSocketBase()->Bind(this->port_, this->ip_.c_str());

	this->ctx_.MysqlRPCServer()->Connect(this->rpc_server_conn_info_);
	this->ctx_.MysqlRPCLog()->Connect(this->rpc_log_conn_info_);
}

void LoginApp::Stop(void)
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

FREEZE_NET::Ctx& LoginApp::GetCtx()
{
	return this->ctx_;
}

bool LoginApp::LoadConfig(std::string file)
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
		const Json::Value src = value[this->service_name_];

		 ip_ = src["ip"].asString();
		 port_ = src["port"].asUInt();


		 const Json::Value rpc_server_db = value["LoginServer"]["db"]["rpc_server"];
		 rpc_server_conn_info_.host = rpc_server_db["host"].asString();
		 rpc_server_conn_info_.user = rpc_server_db["user"].asString();
		 rpc_server_conn_info_.password = rpc_server_db["password"].asString();
		 rpc_server_conn_info_.database = rpc_server_db["database"].asString();
		 rpc_server_conn_info_.port = 3306;


		 const Json::Value rpc_log_db = value["LoginServer"]["db"]["rpc_server"];
		 rpc_log_conn_info_.host = rpc_log_db["host"].asString();
		 rpc_log_conn_info_.user = rpc_log_db["user"].asString();
		 rpc_log_conn_info_.password = rpc_log_db["password"].asString();
		 rpc_log_conn_info_.database = rpc_log_db["database"].asString();
		 rpc_log_conn_info_.port = 3306;
	}

	return true;
}


void LoginApp::AddSession(NetNodeId node, uint32_t serial_num)
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

void LoginApp::RemoveSession(uint32_t serial_num)
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

uint32_t LoginApp::FindSessionSerial(NetNodeId node)
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

NetNodeId LoginApp::FindSessionNode(uint32_t serial_num)
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

void LoginApp::InsertLinkingGateway(NetNodeId node)
{
	if (node.service_ == SERVICE_GATEWAY)
	{
		this->linking_gateway_set_.insert(node);
		std::cout << "create gateway node : " << (uint32_t)node.service_ << " " << node.id_ << std::endl;
	}
}

void LoginApp::DeleteLinkingGateway(NetNodeId node)
{
	if (node.service_ == SERVICE_GATEWAY)
	{
		std::set<NetNodeId>::iterator ite = linking_gateway_set_.find(node);
		if (ite != linking_gateway_set_.end())
		{
			linking_gateway_set_.erase(ite);

			std::cout << "delete gateway node : " << (uint32_t)node.service_ << " " << node.id_ << std::endl;
		}
	}
}

std::set<NetNodeId>& LoginApp::GetLinkingGateway(void)
{
	return this->linking_gateway_set_;
}

std::map<NetNodeId, NetNodeAddress>& LoginApp::GetGatewayAddressMap(void)
{
	return this->gateway_addr_map_;
}

bool LoginApp::ChooseGatewayAddress(uint32_t player_id, NetNodeAddress& address, NetNodeId& gateway_node)
{
	std::map<uint32_t, NetNodeId>::iterator bind_ite = platform_account_binding_gateway_.find(player_id);
	if (bind_ite == platform_account_binding_gateway_.end())
	{
		uint32_t gateway_size = linking_gateway_set_.size();
		if (0 == gateway_size)
		{
			std::cout << "error! linking_gateway_set_ count " << gateway_size << std::endl;
			return false;
		}
		else
		{
			uint32_t index = rand() % gateway_size;

			std::set<NetNodeId>::iterator select_ite = linking_gateway_set_.begin();

			uint32_t count = 0;
			for (std::set<NetNodeId>::iterator ite = linking_gateway_set_.begin();
				ite != linking_gateway_set_.end();
				ite++)
			{
				if (index == count)
				{
					select_ite = ite;

					break;
				}

				count++;
			}

			if (select_ite != linking_gateway_set_.end())
			{
				NetNodeId node = *select_ite;

				std::map<NetNodeId, NetNodeAddress>::iterator find_ite = gateway_addr_map_.find(node);
				if (find_ite != gateway_addr_map_.end())
				{
					address = find_ite->second;
					gateway_node = node;

					platform_account_binding_gateway_[player_id] = node;
					return true;
				}
			}

			std::cout << "error! linking_gateway_set_" << std::endl;
			return false;
		}
	}
	else
	{
		std::set<NetNodeId>::iterator select_ite = linking_gateway_set_.find(bind_ite->second);
		if (select_ite == linking_gateway_set_.end())
		{
			if (linking_gateway_set_.size() > 0)
			{
				select_ite = linking_gateway_set_.begin();

				std::map<NetNodeId, NetNodeAddress>::iterator find_ite = gateway_addr_map_.find(*select_ite);
				if (find_ite != gateway_addr_map_.end())
				{
					address = find_ite->second;
					gateway_node = *select_ite;
					return true;
				}
			}
		}
		else
		{
			std::map<NetNodeId, NetNodeAddress>::iterator find_ite = gateway_addr_map_.find(bind_ite->second);
			if (find_ite != gateway_addr_map_.end())
			{
				address = find_ite->second;
				gateway_node = bind_ite->second;
				return true;
			}
		}

		return false;
	}
}

std::map<uint32_t, NetNodeId>& LoginApp::GetAccountGatewayAddress(void)
{
	return this->platform_account_binding_gateway_;
}

std::map<uint32_t, uint32_t>& LoginApp::GetVerifiedAccount()
{
	return this->verified_account_;
}

std::map<uint32_t, uint32_t>& LoginApp::GetGetVerifiedPlayerSerial(void)
{
	return this->verified_player_serial_;
}

bool LoginApp::SendTo(NetNodeId node, unsigned char* ptr, uint32_t ptr_size, ProtocolHead protocol_haad)
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

void LoginApp::ResponseRequestError(uint32_t serial_num, uint32_t request_opcodes)
{
	ProtocolHead response;
	response.opcode_ = Opcodes::S_REQUEST_ERROR_RESPONSE;


	ByteBuffer response_buffer;
	response_buffer << request_opcodes;

	this->GetCtx().SendBuf(serial_num, response_buffer.RdPos(), response_buffer.Length(), response);
}