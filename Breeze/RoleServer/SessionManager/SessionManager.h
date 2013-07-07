#ifndef __SESSION_MANAGER_SESSION_MANAGER_H__
#define __SESSION_MANAGER_SESSION_MANAGER_H__

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
#include "Game/ServiceType.h"
#include "Net/Ctx.h"

#include "../RoleApp/RoleApp.h"

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
		::RegisterOpcodes(temp, Opcodes::INTERNAL_RESPONSE_TEST, this, &SessionManager::HandleInternalResponse);
		::RegisterOpcodes(temp, Opcodes::INTERNAL_NOTIFY_TEST, this, &SessionManager::HandleInternalNotify);

		::RegisterOpcodes(temp, Opcodes::CLIENT_LINKING_GATEWAY, this, &SessionManager::HandleClientLinkingGateway);
		::RegisterOpcodes(temp, Opcodes::NOTIFY_GATEWAY_LINKING_SERVICE, this, &SessionManager::HandleNotifyGatewayLinkingService);

		return temp;
	}

	////std::map<uint32_t, IDelegate*> GenForwardOpcodes(void)
	////{
	////	std::map<uint32_t, IDelegate*> temp;
	////	return temp;
	////}

	void Login(uint32_t role_id);
	void Logout(uint32_t role_id);

public:

	void HandleNotifyGatewayLinkingService(ByteBuffer* ptr)
	{
		ProtocolHead head;
		(*ptr) >> head;

		NetNodeId service_node;
		(*ptr) >> service_node;

		uint32_t serila_num = head.suspend_session_serial_num_;

		SINGLETON_ROLE_APP->InsertGatewayLinkingService(service_node);
		SINGLETON_ROLE_APP->GetSierialNumServiceMap()[serila_num] = service_node;

		std::cout << "insert node : " << (uint32_t)service_node.service_ << " " << service_node.id_ << std::endl;
	}

	void HandleClientLinkingGateway(ByteBuffer* ptr)
	{
		ProtocolHead head;
		(*ptr) >> head;

		NetNodeId client_node;
		(*ptr) >> client_node;

		NetNodeId gateway_node;
		(*ptr) >> gateway_node;

		SINGLETON_ROLE_APP->GetClientLinkingGateway()[client_node] = gateway_node;
		std::cout << "client node : " << (uint32_t)client_node.service_ << " " << client_node.id_
			<< " -- " << "gateway node : " << (uint32_t)gateway_node.service_ << " " << gateway_node.id_ << std::endl;
	}

	void HandleInternalResponse(ByteBuffer* ptr)
	{

		ProtocolHead head;
		(*ptr) >> head; 

		std::string content;
		(*ptr) >> content;

		std::cout << content << std::endl;
	}

	void HandleInternalNotify(ByteBuffer* ptr)
	{

		ProtocolHead head;
		(*ptr) >> head; 

		std::string content;
		(*ptr) >> content;

		std::cout << content << std::endl;
	}

	void HandleRequest(ByteBuffer* ptr)
	{
		ProtocolHead head;
		(*ptr) >> head;

		std::string content;
		(*ptr) >> content;

		std::cout << "receive content: " << content << std::endl;
		
		ProtocolHead response_head;
		response_head.identification_ = head.identification_;
		response_head.opcode_ = Opcodes::RESPONSE_TEST;
		response_head.destination_service_ = head.origin_service_;
		response_head.destination_session_serial_num_ = head.origin_session_serial_num_;

		ByteBuffer buffer;
		content = content + " response";
		buffer << content;

		SINGLETON_ROLE_APP->GetCtx().SendBuf(head.cur_session_serial_num_, buffer.RdPos(), buffer.Length(), response_head);


		NetNodeId node;
		node.service_ = SERVICE_GATEWAY;
		node.id_ = 1;

		ProtocolHead notify_head;
		notify_head.opcode_ = Opcodes::NOTIFY_TEST;
		notify_head.destination_service_ = head.origin_service_;
		notify_head.destination_session_serial_num_ = head.origin_session_serial_num_;


		ByteBuffer notify_buffer;
		content = "notify";
		notify_buffer << content;

		SINGLETON_ROLE_APP->SendTo(node, notify_buffer.RdPos(), notify_buffer.Length(), notify_head);


		ByteBuffer internal_buffer;

		std::string data = "internal_request";
		internal_buffer << data;

		//发送服务器内部请求到GLOBAL_SERVER
		ProtocolHead internal_head;
		internal_head.opcode_ = Opcodes::INTERNAL_REQUEST_TEST;
		internal_head.origin_service_ = SINGLETON_ROLE_APP->GetNetNodeId().service_;
		internal_head.origin_session_serial_num_ = SINGLETON_ROLE_APP->GetNetNodeId().id_;
		internal_head.destination_service_ = SERVICE_FIGHT;
		internal_head.destination_session_serial_num_ = 2;

		SINGLETON_ROLE_APP->SendTo(node, internal_buffer.RdPos(), internal_buffer.Length(), internal_head);
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

		//SINGLETON_ROLE_APP->SendTo(node, buffer.RdPos(), buffer.Length(), head);
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

		uint32_t serial_num = head.suspend_session_serial_num_;

		//ROLE_SERVER只有和GATEWAY_SERVER直接相连，这里的创建是接收GATEWAY转发的
		if (service == SERVICE_CLIENT)
		{
			SINGLETON_ROLE_APP->GetSessionRoleMap()[serial_num] = id;
			if (SINGLETON_ROLE_APP->GetOnlineRoleSet().count(id) == 0)
			{
				SINGLETON_ROLE_APP->GetOnlineRoleSet().insert(id);
				std::cout << "role id " << id << " login !" << std::endl;

				this->Login(id);
			}
			else
			{
				std::cout << "repeat role id " << id << " login !" << std::endl;
			}
		}
	}

	void HandleClose(ByteBuffer* ptr)
	{
		std::cout << "HandleClose" << std::endl;
		ProtocolHead head;
		(*ptr) >> head;

		uint32_t serial_num = head.suspend_session_serial_num_;

		//处理客户端退出
		std::map<uint32_t, uint32_t>::iterator ite = SINGLETON_ROLE_APP->GetSessionRoleMap().find(serial_num);
		if (ite != SINGLETON_ROLE_APP->GetSessionRoleMap().end())
		{
			if (SINGLETON_ROLE_APP->GetOnlineRoleSet().count(ite->second) != 0)
			{
				SINGLETON_ROLE_APP->GetOnlineRoleSet().erase(ite->second);

				this->Logout(ite->second);

				std::cout << "role id " << ite->second << " logout !" << std::endl;
			}
			else
			{
				std::cout << "repeat role id " << ite->second << " logout !" << std::endl;
			}
		}

		//处理FIGHT_SERVER退出
		std::map<uint32_t, NetNodeId>::iterator fight_ite = SINGLETON_ROLE_APP->GetSierialNumServiceMap().find(serial_num);
		if (fight_ite != SINGLETON_ROLE_APP->GetSierialNumServiceMap().end())
		{
			NetNodeId node;
			node = fight_ite->second;

			std::cout << "delete fight node : " << (uint32_t)node.service_ << " " << node.id_ << std::endl;

			std::map<uint8_t, std::set<NetNodeId> >::iterator temp_ite = SINGLETON_ROLE_APP->GetGatewayLinkingService().find(node.service_);
			if (temp_ite != SINGLETON_ROLE_APP->GetGatewayLinkingService().end())
			{
				if (temp_ite->second.count(node) != 0)
				{
					temp_ite->second.erase(node);

					std::cout << "deleted node : " << (uint32_t)node.service_ << " " << node.id_ << std::endl;
				}
			}
		}
	}
};

#endif