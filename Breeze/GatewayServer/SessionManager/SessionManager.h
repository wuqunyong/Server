#ifndef __SESSION_MANAGER_SESSION_MANAGER_H__
#define __SESSION_MANAGER_SESSION_MANAGER_H__

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
#include "Game/ServiceType.h"
#include "Net/Ctx.h"

#include "../GatewayApp/GatewayApp.h"
#include "../GatewayAuthenticate/GatewayAuthenticate.h"

class SessionManager : public IFacade
{
public:
	SessionManager(void);
	~SessionManager(void);
public:
	virtual void Initialize(void){}
	virtual void Update(void){}

public:
	std::map<uint32_t, IDelegate*> GenOpcodes(void)
	{
		std::map<uint32_t, IDelegate*> temp;

		::RegisterOpcodes(temp, Opcodes::ECHO, this, &SessionManager::HandleEcho);
		::RegisterOpcodes(temp, Opcodes::CREATE_SESSION, this, &SessionManager::HandleCreate);
		::RegisterOpcodes(temp, Opcodes::CLOSE_SESSION, this, &SessionManager::HandleClose);

		::RegisterOpcodes(temp, Opcodes::REQUEST_TEST, this, &SessionManager::HandleRequest);
		::RegisterOpcodes(temp, Opcodes::RESPONSE_TEST, this, &SessionManager::HandleResponse);
		::RegisterOpcodes(temp, Opcodes::NOTIFY_TEST, this, &SessionManager::HandleNotify);


		::RegisterOpcodes(temp, Opcodes::INTERNAL_REQUEST_TEST, this, &SessionManager::HandleInternalRouting);
		::RegisterOpcodes(temp, Opcodes::INTERNAL_RESPONSE_TEST, this, &SessionManager::HandleInternalRouting);
		::RegisterOpcodes(temp, Opcodes::INTERNAL_NOTIFY_TEST, this, &SessionManager::HandleInternalRouting);
		::RegisterOpcodes(temp, Opcodes::L_PLAYER_BINDING_GATEWAY_REQUEST, this, &SessionManager::HandlePlayerBindingGatewayRequest);

		

		return temp;
	}

	//std::map<uint32_t, IDelegate*> GenForwardOpcodes(void)
	//{
	//	std::map<uint32_t, IDelegate*> temp;
	//	return temp;
	//}

public:
	void HandlePlayerBindingGatewayRequest(ByteBuffer* ptr)
	{
		ProtocolHead head;
		(*ptr) >> head; 

		uint32_t serial_num = head.suspend_session_serial_num_;

		uint32_t player_id = 0;
		(*ptr) >> player_id;

		uint32_t role_count = 0;
		(*ptr) >> role_count;

		std::set<uint32_t> role_vec;
		for (uint32_t i = 0; i < role_count; i++)
		{
			uint32_t role_id = 0;
			(*ptr) >> role_id;

			role_vec.insert(role_id);
		}

		std::string validate;
		(*ptr) >> validate;

		SINGLETON_GATEWAY_AUTHENTICATE->GetKeyRoleSet()[validate] = role_vec;

		//回复LOGIN
		ProtocolHead response_head;
		response_head.opcode_ = Opcodes::G_PLAYER_BINDING_GATEWAY_RESPONSE;

		ByteBuffer response_buffer;
		response_buffer << player_id;

		NetNodeAddress node_address = SINGLETON_GATEWAY_APP->GetNetNodeAddress();
		response_buffer << node_address;
		response_buffer << validate;

		uint32_t temp_count = role_vec.size();
		response_buffer << temp_count;
		for (std::set<uint32_t>::iterator ite = role_vec.begin();
			ite != role_vec.end();
			ite++)
		{
			response_buffer << *ite;
		}

		SINGLETON_GATEWAY_APP->GetCtx().SendBuf(serial_num, response_buffer.RdPos(), response_buffer.Length(), response_head);
	}

	//数据包的发起端，不是GATEWAY_SERVER，则使用路由协议转发
	void HandleInternalRouting(ByteBuffer* ptr)
	{
		ProtocolHead head;
		bool result = (*ptr).Peek<ProtocolHead>(head);
		if (result)
		{
			uint32_t serial_num = head.suspend_session_serial_num_;
			NetNodeId node = SINGLETON_GATEWAY_APP->FindSessionNode(serial_num);

			head.origin_service_ = node.service_;
			head.origin_session_serial_num_ = node.id_;

			*((ProtocolHead*)ptr->RdPos()) = head;

			ByteBuffer routing_buffer;

			ProtocolHead routing_head;
			routing_head.opcode_ = Opcodes::ROUTING;

			NetNodeId target_node;
			target_node.service_ = head.destination_service_;
			target_node.id_ = head.destination_session_serial_num_;

			SINGLETON_GATEWAY_APP->SendTo(target_node, ptr->RdPos(), ptr->Length(), routing_head);
		}
	}

	void HandleRequest(ByteBuffer* ptr)
	{
		ProtocolHead head;
		bool result = (*ptr).Peek<ProtocolHead>(head);
		if (result)
		{
			uint32_t serial_num = head.suspend_session_serial_num_;
			NetNodeId node = SINGLETON_GATEWAY_APP->FindSessionNode(serial_num);

			std::cout << "receive opcodes " << head.opcode_ << std::endl;

			head.origin_service_ = node.service_;
			head.origin_session_serial_num_ = node.id_;

			*((ProtocolHead*)ptr->RdPos()) = head;


			ProtocolHead routing_head;
			routing_head.opcode_ = Opcodes::ROUTING;

			NetNodeId target_node;
			target_node.service_ = SERVICE_ROLE;
			target_node.id_ = 1;
			SINGLETON_GATEWAY_APP->SendTo(target_node, ptr->RdPos(), ptr->Length(), routing_head);
		}

	}

	void HandleResponse(ByteBuffer* ptr)
	{

		ProtocolHead head;
		(*ptr) >> head; 

		NetNodeId node;
		node.service_ = head.destination_service_;
		node.id_ = head.destination_session_serial_num_;

		SINGLETON_GATEWAY_APP->SendTo(node, ptr->RdPos(), ptr->Length(), head);
	}

	void HandleNotify(ByteBuffer* ptr)
	{

		ProtocolHead head;
		(*ptr) >> head; 

		NetNodeId node;
		node.service_ = head.destination_service_;
		node.id_ = head.destination_session_serial_num_;

		SINGLETON_GATEWAY_APP->SendTo(node, ptr->RdPos(), ptr->Length(), head);
	}


	void HandleEcho(ByteBuffer* ptr)
	{
		ProtocolHead head;
		(*ptr) >> head;


		uint32_t count;
		std::string content;

		(*ptr) >> count;
		(*ptr) >> content;

		count++;

		//std::cout << "count " << count << std::endl;

		//head.opcode_ = Opcodes::ECHO;
		//NetNodeId node;
		//node.service_ = SERVICE_LOGIN;
		//node.id_ = 1;

		//ByteBuffer buffer;
		//buffer << count;
		//buffer << content;

		//SINGLETON_GATEWAY_APP->SendTo(node, buffer.RdPos(), buffer.Length(), head);
	}

	void HandleCreate(ByteBuffer* ptr)
	{
		ProtocolHead head;
		(*ptr) >> head;

		uint8_t service;
		uint32_t id;
		std::string validate;

		(*ptr) >> service;
		(*ptr) >> id;
		(*ptr) >> validate;

		switch(service)
		{
		case SERVICE_CLIENT:
			{
				if (validate != "ClientServer")
				{
					std::cout << "ClientServer validate error!" << std::endl;
					SINGLETON_GATEWAY_APP->GetCtx().Close(head.suspend_session_serial_num_);
					return;
				}
				break;
			}
		case SERVICE_ROLE:
			{
				if (validate != "RoleServer")
				{
					std::cout << "RoleServer validate error!" << std::endl;
					SINGLETON_GATEWAY_APP->GetCtx().Close(head.suspend_session_serial_num_);
					return;
				}
				break;
			}
		case SERVICE_FIGHT:
			{
				if (validate != "FightServer")
				{
					std::cout << "FightServer validate error!" << std::endl;
					SINGLETON_GATEWAY_APP->GetCtx().Close(head.suspend_session_serial_num_);
					return;
				}
				break;
			}
		default:
			{
				std::cout << "validate error ! "  << (uint32_t)service << " "  << id << std::endl;
				SINGLETON_GATEWAY_APP->GetCtx().Close(head.suspend_session_serial_num_);
				return;

				break;
			}
		}

		NetNodeId node(service,id);

		uint32_t serial_num = head.suspend_session_serial_num_;

		SINGLETON_GATEWAY_APP->AddSession(node, serial_num);
		SINGLETON_GATEWAY_APP->InsertInternalService(node);

		switch(node.service_)
		{
		case SERVICE_CLIENT:
			{
				//客户端创建SESSION连接，通知ROLE_SERVER, FIGHT_SERVER
				for (std::map<uint32_t, std::set<NetNodeId> >::iterator ite = SINGLETON_GATEWAY_APP->GetInternalService().begin();
					ite != SINGLETON_GATEWAY_APP->GetInternalService().end();
					ite++)
				{
					if (ite->first == SERVICE_ROLE || ite->first == SERVICE_FIGHT)
					{				
						for (std::set<NetNodeId>::iterator node_ite = ite->second.begin();
							node_ite != ite->second.end();
							node_ite++)
						{
							ProtocolHead routing_head;
							routing_head.opcode_ = Opcodes::ROUTING;

							ProtocolHead notify_head = head;
							notify_head.opcode_ = Opcodes::CLIENT_LINKING_GATEWAY;

							NetNodeId target_node;
							target_node.service_ = node_ite->service_;
							target_node.id_ = node_ite->id_;

							ByteBuffer notify_buffer;
							notify_buffer << node;

							NetNodeId linking_node = SINGLETON_GATEWAY_APP->GetNodeId(); 
							notify_buffer << linking_node;

							std::string package_buffer = SINGLETON_GATEWAY_APP->GetCtx().BuildPackage(notify_head, notify_buffer.RdPos(), notify_buffer.Length());

							SINGLETON_GATEWAY_APP->SendTo(target_node, (unsigned char*)package_buffer.c_str(), package_buffer.length(), routing_head);
						}
					}
				}

				//转发客户端的SESSION创建连接给，ROLE_SERVER, SERVICE_FIGHT
				for (std::map<uint32_t, std::set<NetNodeId> >::iterator ite = SINGLETON_GATEWAY_APP->GetInternalService().begin();
					ite != SINGLETON_GATEWAY_APP->GetInternalService().end();
					ite++)
				{
					if (ite->first == SERVICE_ROLE || ite->first == SERVICE_FIGHT)
					{				
						for (std::set<NetNodeId>::iterator node_ite = ite->second.begin();
							node_ite != ite->second.end();
							node_ite++)
						{
							//复制收到的数据包
							ByteBuffer duplicate_buffer;
							duplicate_buffer << head;
							duplicate_buffer << service;
							duplicate_buffer << id;
							duplicate_buffer << validate;

							ProtocolHead routing_head;
							routing_head.opcode_ = Opcodes::ROUTING;

							NetNodeId target_node;
							target_node.service_ = node_ite->service_;
							target_node.id_ = node_ite->id_;
							SINGLETON_GATEWAY_APP->SendTo(target_node, duplicate_buffer.RdPos(), duplicate_buffer.Length(), routing_head);
						}
					}
				}
				break;
			}
		case SERVICE_ROLE:
		case SERVICE_FIGHT:
			{
				//把新注册的node，广播给已注册的节点
				for (std::map<uint32_t, std::set<NetNodeId> >::iterator ite = SINGLETON_GATEWAY_APP->GetInternalService().begin();
					ite != SINGLETON_GATEWAY_APP->GetInternalService().end();
					ite++)
				{
					if (ite->first == SERVICE_ROLE || ite->first == SERVICE_FIGHT)
					{				
						for (std::set<NetNodeId>::iterator node_ite = ite->second.begin();
							node_ite != ite->second.end();
							node_ite++)
						{
							ProtocolHead routing_head;
							routing_head.opcode_ = Opcodes::ROUTING;

							ProtocolHead notify_head = head;
							notify_head.opcode_ = Opcodes::NOTIFY_GATEWAY_LINKING_SERVICE;

							NetNodeId target_node;
							target_node.service_ = node_ite->service_;
							target_node.id_ = node_ite->id_;

							ByteBuffer notify_buffer;
							notify_buffer << node;

							std::string package_buffer = SINGLETON_GATEWAY_APP->GetCtx().BuildPackage(notify_head, notify_buffer.RdPos(), notify_buffer.Length());

							SINGLETON_GATEWAY_APP->SendTo(target_node, (unsigned char*)package_buffer.c_str(), package_buffer.length(), routing_head);
						}
					}
				}

				//把已注册的节点，同步给新注册的node
				for (std::map<uint32_t, std::set<NetNodeId> >::iterator ite = SINGLETON_GATEWAY_APP->GetInternalService().begin();
					ite != SINGLETON_GATEWAY_APP->GetInternalService().end();
					ite++)
				{
					if (ite->first == SERVICE_ROLE || ite->first == SERVICE_FIGHT)
					{				
						for (std::set<NetNodeId>::iterator node_ite = ite->second.begin();
							node_ite != ite->second.end();
							node_ite++)
						{
							ProtocolHead routing_head;
							routing_head.opcode_ = Opcodes::ROUTING;

							ProtocolHead notify_head = head;
							notify_head.opcode_ = Opcodes::NOTIFY_GATEWAY_LINKING_SERVICE;

							NetNodeId already_node;
							already_node.service_ = node_ite->service_;
							already_node.id_ = node_ite->id_;

							ByteBuffer notify_buffer;
							notify_buffer << already_node;

							std::string package_buffer = SINGLETON_GATEWAY_APP->GetCtx().BuildPackage(notify_head, notify_buffer.RdPos(), notify_buffer.Length());

							SINGLETON_GATEWAY_APP->SendTo(node, (unsigned char*)package_buffer.c_str(), package_buffer.length(), routing_head);
						}
					}
				}
				break;
			}

		default:
			{
				std::cout << "register service invalid !" << __FILE__ << " " << __LINE__ << std::endl;
				break;
			}
		}

	}

	void HandleClose(ByteBuffer* ptr)
	{
		std::cout << "HandleClose" << std::endl;

		ProtocolHead head;
		bool result = (*ptr).Peek<ProtocolHead>(head);
		if (result)
		{
			uint32_t serial_num = head.suspend_session_serial_num_;

			NetNodeId node = SINGLETON_GATEWAY_APP->FindSessionNode(serial_num);

			//客户端关闭SESSION连接，通知ROLE_SERVER, FIGHT_SERVER
			if (node.service_ == SERVICE_CLIENT)
			{
				for (std::map<uint32_t, std::set<NetNodeId> >::iterator ite = SINGLETON_GATEWAY_APP->GetInternalService().begin();
					ite != SINGLETON_GATEWAY_APP->GetInternalService().end();
					ite++)
				{
					if (ite->first == SERVICE_ROLE || ite->first == SERVICE_FIGHT)
					{				
						for (std::set<NetNodeId>::iterator node_ite = ite->second.begin();
							node_ite != ite->second.end();
							node_ite++)
						{
							ProtocolHead routing_head;
							routing_head.opcode_ = Opcodes::ROUTING;

							NetNodeId target_node;
							target_node.service_ = node_ite->service_;
							target_node.id_ = node_ite->id_;
							SINGLETON_GATEWAY_APP->SendTo(target_node, ptr->RdPos(), ptr->Length(), routing_head);
						}
					}
				}
			}
			else if (node.service_ == SERVICE_FIGHT)
			{
				//如果FIGHT_SERVER挂掉，通知ROLE_SERVER
				for (std::map<uint32_t, std::set<NetNodeId> >::iterator ite = SINGLETON_GATEWAY_APP->GetInternalService().begin();
					ite != SINGLETON_GATEWAY_APP->GetInternalService().end();
					ite++)
				{
					if (ite->first == SERVICE_ROLE )
					{				
						for (std::set<NetNodeId>::iterator node_ite = ite->second.begin();
							node_ite != ite->second.end();
							node_ite++)
						{
							ProtocolHead routing_head;
							routing_head.opcode_ = Opcodes::ROUTING;

							NetNodeId target_node;
							target_node.service_ = node_ite->service_;
							target_node.id_ = node_ite->id_;
							SINGLETON_GATEWAY_APP->SendTo(target_node, ptr->RdPos(), ptr->Length(), routing_head);
						}
					}
				}
			}
			else
			{
				std::cout << "node not find !" << std::endl;
			}

			SINGLETON_GATEWAY_APP->DeleteInternalService(node);
		}
	}
};

#endif