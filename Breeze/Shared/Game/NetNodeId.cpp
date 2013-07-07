#include "NetNodeId.h"

NetNodeId::NetNodeId(uint8_t service, uint32_t id)
{
	this->service_ = service;
	this->id_ = id;
}

NetNodeId::~NetNodeId(void)
{

}

NetNodeAddress::NetNodeAddress()
{
	this->port_ = 0;
}

NetNodeAddress::~NetNodeAddress()
{

}

ByteBuffer& operator >> (ByteBuffer& stream, NetNodeId& data)
{
	stream >> data.service_;
	stream >> data.id_;

	return stream;
}

ByteBuffer& operator << (ByteBuffer& stream, NetNodeId data)
{
	stream << data.service_;
	stream << data.id_;

	return stream;
}

ByteBuffer& operator >> (ByteBuffer& stream, NetNodeAddress& data)
{
	stream >> data.ip_;
	stream >> data.port_;

	return stream;
}

ByteBuffer& operator << (ByteBuffer& stream, NetNodeAddress data)
{
	stream << data.ip_;
	stream << data.port_;

	return stream;
}