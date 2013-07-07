#include "SessionManager.h"


SessionManager::SessionManager(void)
{

}

SessionManager::~SessionManager(void)
{

}

void SessionManager::Initialize(void)
{

}

void SessionManager::Update(void)
{

}

void SessionManager::HandlePlayerBindingGatewayResponse(ByteBuffer* ptr)
{
	ProtocolHead head;
	(*ptr) >> head;

	uint32_t player_id = 0;
	(*ptr) >> player_id;

	NetNodeAddress node_address;
	(*ptr) >> node_address;

	std::string validate;
	(*ptr) >> validate;

	uint32_t role_count = 0;
	(*ptr) >> role_count;

	std::set<uint32_t> role_id_set;
	for (uint32_t i = 0; i < role_count; i++)
	{
		uint32_t role_id = 0;
		(*ptr) >> role_id;

		role_id_set.insert(role_id);
	}

	std::map<uint32_t, uint32_t>::iterator player_ite = SINGLETON_LOGIN_APP->GetGetVerifiedPlayerSerial().find(player_id);
	if (player_ite != SINGLETON_LOGIN_APP->GetGetVerifiedPlayerSerial().end())
	{
		ProtocolHead notify;
		notify.opcode_ = Opcodes::S_GATEWAY_ADDRESS_NOTIFY;

		ByteBuffer notify_buffer;
		notify_buffer << node_address;

		notify_buffer << validate;
		notify_buffer << role_count;
		for (std::set<uint32_t>::iterator ite = role_id_set.begin();
			ite != role_id_set.end();
			ite++)
		{
			notify_buffer << *ite;
		}

		SINGLETON_LOGIN_APP->GetCtx().SendBuf(player_ite->second, notify_buffer.RdPos(), notify_buffer.Length(), notify);
	}

}

void SessionManager::HandleGetRoleRequest(ByteBuffer* ptr)
{
	ProtocolHead head;
	(*ptr) >> head;

	uint32_t serial_num = head.suspend_session_serial_num_;

	std::map<uint32_t, uint32_t>::iterator ite = SINGLETON_LOGIN_APP->GetVerifiedAccount().find(serial_num);
	if (ite != SINGLETON_LOGIN_APP->GetVerifiedAccount().end())
	{
		uint32_t player_id = ite->second;

		//查看缓存中是否有玩家信息
		std::map<uint32_t, std::set<RoleRegisterInfo> >::iterator
			cache_ite = SINGLETON_LOGIN_PLAYER_MANAGER->GetPlayerInfo().find(player_id);
		if (cache_ite == SINGLETON_LOGIN_PLAYER_MANAGER->GetPlayerInfo().end())
		{
			std::stringstream ss;
			ss << "SELECT "
				<< "tb_player_role.role_id, "
				<< "tb_player_role.name, "
				<< "tb_player_role.sex, "
				<< "tb_player_role.profession "
				<< "FROM "
				<< "tb_player_role "
				<< "WHERE "
				<< "tb_player_role.player_id =  '" << player_id << "'; ";


			std::string query_statement;
			query_statement = ss.str();

			MethodClosureEx1<SessionManager>* callback_cmd = NULL;
			callback_cmd = new MethodClosureEx1<SessionManager>(this, &SessionManager::MySQLRPCGetRoleRequest, true);
			callback_cmd->byte_buffer_ << head;
			callback_cmd->byte_buffer_ << player_id;

			SINGLETON_LOGIN_APP->GetCtx().MysqlRPCClient()->SendRPC(query_statement.c_str(), callback_cmd, serial_num);
		}
		else
		{
			SINGLETON_LOGIN_PLAYER_MANAGER->ResponseGetRole(serial_num, player_id, cache_ite->second);
		}
	}
	else
	{
		SINGLETON_LOGIN_APP->ResponseRequestError(serial_num, head.opcode_);
	}
}

void SessionManager::MySQLRPCGetRoleRequest(ByteBuffer& ref_data, ResultSet* query_result)
{
	ProtocolHead head;
	ref_data >> head;

	uint32_t player_id = 0;
	ref_data >> player_id;

	uint32_t serial_num = head.suspend_session_serial_num_;

	std::set<RoleRegisterInfo> role_set;

	if (NULL != query_result)
	{
		while (query_result->MoveNext())
		{
			RoleRegisterInfo role_item;

			if (*query_result >> role_item.role_id_
				&& *query_result >> role_item.name_
				&& *query_result >> role_item.sex_
				&& *query_result >> role_item.profession_)
			{
				role_set.insert(role_item);
			}
		}
	}

	SINGLETON_LOGIN_PLAYER_MANAGER->GetPlayerInfo()[player_id] = role_set;

	SINGLETON_LOGIN_PLAYER_MANAGER->ResponseGetRole(serial_num, player_id, role_set);
}

void SessionManager::HandlePlatformAccountLoginRequest(ByteBuffer* ptr)
{
	ProtocolHead head;
	(*ptr) >> head;

	std::string account_name;
	(*ptr) >> account_name;

	std::string passwd;
	(*ptr) >> passwd;

	std::stringstream ss;
	ss << "SELECT "
		<< "tb_player.id "
		<< "FROM "
		<< "tb_player "
		<< "WHERE "
		<< "tb_player.login =  '" << account_name << "' AND "
		<< "tb_player.`password` =  '" << passwd << "'; ";

	std::string query_statement;
	query_statement = ss.str();

	MethodClosureEx1<SessionManager>* callback_cmd = NULL;
	callback_cmd = new MethodClosureEx1<SessionManager>(this, &SessionManager::MySQLRPCAuthenticateAccount, true);

	callback_cmd->byte_buffer_ << head;
	callback_cmd->byte_buffer_ << account_name;
	callback_cmd->byte_buffer_ << passwd;

	uint32_t serial_num = head.suspend_session_serial_num_;
	SINGLETON_LOGIN_APP->GetCtx().MysqlRPCClient()->SendRPC(query_statement.c_str(), callback_cmd, serial_num);
}

void SessionManager::MySQLRPCAuthenticateAccount(ByteBuffer& ref_data, ResultSet* query_result)
{
	ProtocolHead head;
	ref_data >> head;

	uint32_t serial_num = head.suspend_session_serial_num_;

	std::string account_name;
	ref_data >> account_name;

	std::string passwd;
	ref_data >> passwd;

	uint32_t error_code = ERROR_CODE_ACCOUNT_PASSWORD_ERROR;

	if (NULL != query_result)
	{
		uint32_t player_id = 0;

		while (query_result->MoveNext())
		{
			if (*query_result >> player_id)
			{
				error_code = ERROR_CODE_SUCCESS;
			}
		}

		if (ERROR_CODE_SUCCESS == error_code)
		{
			//记录已验证的玩家
			SINGLETON_LOGIN_APP->GetVerifiedAccount()[serial_num] = player_id;
			SINGLETON_LOGIN_APP->GetGetVerifiedPlayerSerial()[player_id] = serial_num;
		}
	}

	ProtocolHead response;
	response.opcode_ = Opcodes::PLATFORM_ACCOUNT_LOGIN_RESPONSE;

	ByteBuffer response_buffer;
	response_buffer << error_code;

	SINGLETON_LOGIN_APP->GetCtx().SendBuf(serial_num, response_buffer.RdPos(), response_buffer.Length(), response);
}

void SessionManager::HandleRegisterGatewayAddress(ByteBuffer* ptr)
{
	ProtocolHead head;
	(*ptr) >> head;

	NetNodeId node;
	(*ptr) >> node;

	NetNodeAddress node_address;
	(*ptr) >> node_address;

	SINGLETON_LOGIN_APP->GetGatewayAddressMap()[node] = node_address;
}

void SessionManager::HandleEcho(ByteBuffer* ptr)
{
	ProtocolHead head;
	(*ptr) >> head;

	uint32_t count;
	std::string content;

	(*ptr) >> count;
	(*ptr) >> content;
}

void SessionManager::HandleCreate(ByteBuffer* ptr)
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
				SINGLETON_LOGIN_APP->GetCtx().Close(head.suspend_session_serial_num_);
				return;
			}
			break;
		}
	case SERVICE_GATEWAY:
		{
			if (validate != "GatewayServer")
			{
				std::cout << "GatewayServer validate error!" << std::endl;
				SINGLETON_LOGIN_APP->GetCtx().Close(head.suspend_session_serial_num_);
				return;
			}
			break;
		}
	default:
		{
			std::cout << "validate error ! "  << (uint32_t)service << " "  << id << std::endl;
			SINGLETON_LOGIN_APP->GetCtx().Close(head.suspend_session_serial_num_);
			return;

			break;
		}
	}

	NetNodeId node(service,id);

	uint32_t serial_num = head.suspend_session_serial_num_;
	SINGLETON_LOGIN_APP->AddSession(node, serial_num);

	SINGLETON_LOGIN_APP->InsertLinkingGateway(node);
}

void SessionManager::HandleClose(ByteBuffer* ptr)
{
	ProtocolHead head;
	(*ptr) >> head;

	uint32_t serial_num = head.suspend_session_serial_num_;
	NetNodeId node = SINGLETON_LOGIN_APP->FindSessionNode(serial_num);

	SINGLETON_LOGIN_APP->DeleteLinkingGateway(node);

	std::map<uint32_t, uint32_t>::iterator ite = SINGLETON_LOGIN_APP->GetVerifiedAccount().find(serial_num);
	if (ite != SINGLETON_LOGIN_APP->GetVerifiedAccount().end())
	{
		std::map<uint32_t, uint32_t>::iterator player_ite = SINGLETON_LOGIN_APP->GetGetVerifiedPlayerSerial().find(ite->second);
		if (player_ite != SINGLETON_LOGIN_APP->GetGetVerifiedPlayerSerial().end())
		{
			SINGLETON_LOGIN_APP->GetGetVerifiedPlayerSerial().erase(player_ite);
		}

		SINGLETON_LOGIN_APP->GetVerifiedAccount().erase(ite);
	}
}