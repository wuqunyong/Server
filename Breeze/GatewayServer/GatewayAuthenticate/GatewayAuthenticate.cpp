#include "GatewayAuthenticate.h"

GatewayAuthenticate::GatewayAuthenticate(void)
{

}

GatewayAuthenticate::~GatewayAuthenticate(void)
{

}

std::map<std::string, std::set<uint32_t> >& GatewayAuthenticate::GetKeyRoleSet(void)
{
	return this->key_role_set_;
}