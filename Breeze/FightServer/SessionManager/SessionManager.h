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

#include "../FightApp/FightApp.h"

class SessionManager : public IFacade
{
public:
	SessionManager(void);
	~SessionManager(void);
public:
	virtual void Initialize(void){}
	virtual void Update(void);

public:
	std::map<uint32_t, IDelegate*> GenOpcodes(void)
	{
		std::map<uint32_t, IDelegate*> temp;

		::RegisterOpcodes(temp, Opcodes::ECHO, this, &SessionManager::HandleEcho);
		::RegisterOpcodes(temp, Opcodes::CREATE_SESSION, this, &SessionManager::HandleCreate);
		::RegisterOpcodes(temp, Opcodes::CLOSE_SESSION, this, &SessionManager::HandleClose);

		::RegisterOpcodes(temp, Opcodes::INTERNAL_REQUEST_TEST, this, &SessionManager::HandleInternalRequest);
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

		SINGLETON_FIGHT_APP->InsertGatewayLinkingService(service_node);
	}

	void HandleClientLinkingGateway(ByteBuffer* ptr)
	{
		ProtocolHead head;
		(*ptr) >> head;

		NetNodeId client_node;
		(*ptr) >> client_node;

		NetNodeId gateway_node;
		(*ptr) >> gateway_node;

		SINGLETON_FIGHT_APP->GetClientLinkingGateway()[client_node] = gateway_node;
	}

	void HandleInternalRequest(ByteBuffer* ptr)
	{
		ProtocolHead head;
		(*ptr) >> head;

		std::string content;
		(*ptr) >> content;

		std::cout << content << std::endl;

		ProtocolHead response_head;

		response_head.opcode_ = Opcodes::INTERNAL_RESPONSE_TEST;
		//response_head.destination_service_ = head.origin_service_;
		//response_head.destination_session_serial_num_ = head.origin_session_serial_num_;

		ByteBuffer buffer;
		content = "fight internal response";
		buffer << content;

		//SINGLETON_FIGHT_APP->GetCtx().SendBuf(head.cur_session_serial_num_, buffer.RdPos(), buffer.Length(), response_head);
		SINGLETON_FIGHT_APP->SendToRoleServer(buffer.RdPos(), buffer.Length(), response_head);




		//NetNodeId node;
		//node.service_ = SERVICE_GATEWAY;
		//node.id_ = 1;

		ProtocolHead notify_head;
		notify_head.opcode_ = Opcodes::INTERNAL_NOTIFY_TEST;
		//notify_head.destination_service_ = head.origin_service_;
		//notify_head.destination_session_serial_num_ = head.origin_session_serial_num_;


		ByteBuffer notify_buffer;
		content = "fight internal notify";
		notify_buffer << content;

		//SINGLETON_FIGHT_APP->SendTo(node, notify_buffer.RdPos(), notify_buffer.Length(), notify_head);
		SINGLETON_FIGHT_APP->SendToRoleServer(notify_buffer.RdPos(), notify_buffer.Length(), notify_head);
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

		std::cout << "count " << count << std::endl;

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

		NetNodeId node(service,id);

		uint32_t serial_num = head.suspend_session_serial_num_;

		//ROLE_SERVER只有和GATEWAY_SERVER直接相连，这里的创建是接收GATEWAY转发的
		if (service == SERVICE_CLIENT)
		{
			SINGLETON_FIGHT_APP->GetSessionRoleMap()[serial_num] = id;
			if (SINGLETON_FIGHT_APP->GetOnlineRoleSet().count(id) == 0)
			{
				SINGLETON_FIGHT_APP->GetOnlineRoleSet().insert(id);
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
		std::map<uint32_t, uint32_t>::iterator ite = SINGLETON_FIGHT_APP->GetSessionRoleMap().find(serial_num);
		if (ite != SINGLETON_FIGHT_APP->GetSessionRoleMap().end())
		{
			if (SINGLETON_FIGHT_APP->GetOnlineRoleSet().count(ite->second) != 0)
			{
				SINGLETON_FIGHT_APP->GetOnlineRoleSet().erase(ite->second);

				this->Logout(ite->second);

				std::cout << "role id " << ite->second << " logout !" << std::endl;
			}
			else
			{
				std::cout << "repeat role id " << ite->second << " logout !" << std::endl;
			}
		}
		else
		{
			//处理网关服务退出
			NetNodeId node;
			node = SINGLETON_FIGHT_APP->FindSessionNode(serial_num);
			if (node.service_ == SERVICE_GATEWAY)
			{
				SINGLETON_FIGHT_APP->RemoveSession(serial_num);
				std::cout << "deleted gateway node : " << (uint32_t)node.service_ << " " << node.id_ << std::endl;;
			}
		}

		

	}
};

#endif