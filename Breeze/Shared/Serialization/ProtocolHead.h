/*
Copyright (c) 2007-2011 iMatix Corporation
Copyright (c) 2007-2011 Other contributors as noted in the AUTHORS file

This file is part of 0MQ.

0MQ is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

0MQ is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SHARED_SERIALIZATION_PROTOCOL_HEAD_H__
#define __SHARED_SERIALIZATION_PROTOCOL_HEAD_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>

#include "ByteBuffer.h"

#pragma pack(1)

struct ProtocolHead
{
	ProtocolHead(void)
	{
		this->total_length_ = 0;
		this->identification_ = 0;
		this->version_ = 0;
		this->type_of_service_ = 0;

		this->opcode_ = 0;

		this->origin_service_ = 0;
		this->origin_session_serial_num_ = 0; 
		this->destination_service_ = 0;
		this->destination_session_serial_num_ = 0;

		this->cur_session_serial_num_ = 0;
		this->suspend_session_serial_num_ = 0;
	}

	uint32_t total_length_;
	uint32_t identification_;
	uint8_t version_;
	uint8_t type_of_service_;
	uint32_t opcode_;
	uint8_t origin_service_;            //原始的SESSION服务类型
	uint32_t origin_session_serial_num_; //原始SESSION序列号
	uint8_t destination_service_;                //目的地的服务类型
	uint32_t destination_session_serial_num_;     //目的地的SESSION序列号
	uint32_t cur_session_serial_num_;    //当前通信SESSION序列号
	uint32_t suspend_session_serial_num_; //调用RPC请求时应暂停的SESSION序列号
};

#pragma pack()

extern ByteBuffer& operator >> (ByteBuffer& stream, ProtocolHead& data);
extern ByteBuffer& operator << (ByteBuffer& stream, ProtocolHead data);

#endif
