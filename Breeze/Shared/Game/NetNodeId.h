#ifndef __GAME_NET_NODE_ID_H__
#define  __GAME_NET_NODE_ID_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>
#include <utility>      // std::rel_ops

#include "../Net/Fd.h"
#include "../Serialization/ByteBuffer.h"

using namespace std::rel_ops;

class NetNodeId
{
public:
	NetNodeId(uint8_t service = 0, uint32_t id = 0);
	~NetNodeId(void);

	bool operator==(const NetNodeId& rhs) const 
	{
		return (service_ == rhs.service_) && (id_ == rhs.id_);
	}
	bool operator< (const NetNodeId& rhs) const 
	{
		return  (this->service_ < rhs.service_ ||
			!(rhs.service_ < this->service_) && this->id_ < rhs.id_);
	}

public:
	uint8_t service_;
	uint32_t id_;
};

class NetNodeAddress
{
public:
	NetNodeAddress();
	~NetNodeAddress();

public:
	std::string ip_;

#ifdef WIN32
	u_short port_;
#else
	uint16_t port_;
#endif

};

extern ByteBuffer& operator >> (ByteBuffer& stream, NetNodeId& data);
extern ByteBuffer& operator << (ByteBuffer& stream, NetNodeId data);

extern ByteBuffer& operator >> (ByteBuffer& stream, NetNodeAddress& data);
extern ByteBuffer& operator << (ByteBuffer& stream, NetNodeAddress data);

#endif