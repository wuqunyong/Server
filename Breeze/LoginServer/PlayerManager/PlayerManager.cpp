#include "PlayerManager.h"

#include <time.h>

#include "../LoginApp/LoginApp.h"
#include "Utils/MD5.h"

PlayerManager::PlayerManager(void)
{

}

PlayerManager::~PlayerManager(void)
{

}

void PlayerManager::ChooseGatewayAddress(uint32_t player_id, std::set<uint32_t> role_set)
{
	NetNodeAddress address;
	NetNodeId gateway_node;

	bool result = SINGLETON_LOGIN_APP->ChooseGatewayAddress(player_id, address, gateway_node);
	if (result)
	{
		ProtocolHead request_head;
		request_head.opcode_ = Opcodes::L_PLAYER_BINDING_GATEWAY_REQUEST;

		ByteBuffer request_buffer;

		uint32_t role_count = role_set.size();
		request_buffer << player_id;
		request_buffer << role_count;

		for (std::set<uint32_t>::iterator ite = role_set.begin();
			ite != role_set.end();
			ite++)
		{
			request_buffer << *ite;
		}

		std::stringstream ss;
		ss << player_id;

		uint32_t timer;
		timer = time(NULL);
		ss << timer;

		std::string key;
		key = ss.str();

		std::string validate = Md5String(key);
		request_buffer << validate;

		SINGLETON_LOGIN_APP->SendTo(gateway_node, request_buffer.RdPos(), request_buffer.Length(), request_head);
	}
}

void PlayerManager::ResponseGetRole(uint32_t serial_num, uint32_t player_id, std::set<RoleRegisterInfo> role_set)
{
	ProtocolHead response;
	response.opcode_ = Opcodes::S_GET_ROLE_RESPONSE;

	ByteBuffer response_buffer;

	std::set<uint32_t> role_id_set;

	uint32_t role_count = role_set.size();
	response_buffer << role_count;

	for (std::set<RoleRegisterInfo>::iterator ite = role_set.begin();
		ite != role_set.end();
		ite++)
	{
		response_buffer << *ite;

		role_id_set.insert(ite->role_id_);
	}

	SINGLETON_LOGIN_APP->GetCtx().SendBuf(serial_num, response_buffer.RdPos(), response_buffer.Length(), response);

	if (role_id_set.size() > 0)
	{
		SINGLETON_LOGIN_PLAYER_MANAGER->ChooseGatewayAddress(player_id, role_id_set);
	}
}

std::map<uint32_t, std::set<RoleRegisterInfo> >& PlayerManager::GetPlayerInfo(void)
{
	return this->player_info_;
}

ByteBuffer& operator >> (ByteBuffer& stream, RoleRegisterInfo& data)
{
	stream >> data.role_id_;
	stream >> data.name_;
	stream >> data.sex_;
	stream >> data.profession_;

	return stream;
}

ByteBuffer& operator << (ByteBuffer& stream, RoleRegisterInfo data)
{
	stream << data.role_id_;
	stream << data.name_;
	stream << data.sex_;
	stream << data.profession_;

	return stream;
}