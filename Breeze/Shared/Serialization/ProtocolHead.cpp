#include "ProtocolHead.h"

 ByteBuffer& operator >> (ByteBuffer& stream, ProtocolHead& data)
 {
	  stream >> data.total_length_;
	  stream >> data.identification_;
	  stream >> data.version_;
	  stream >> data.type_of_service_;
	  stream >> data.opcode_;
	  stream >> data.origin_service_;
	  stream >> data.origin_session_serial_num_; 
	  stream >> data.destination_service_;
	  stream >> data.destination_session_serial_num_;
	  stream >> data.cur_session_serial_num_;
	  stream >> data.suspend_session_serial_num_;

	 return stream;
 }

 ByteBuffer& operator << (ByteBuffer& stream, ProtocolHead data)
 {
	 stream << data.total_length_;
	 stream << data.identification_;
	 stream << data.version_;
	 stream << data.type_of_service_;
	 stream << data.opcode_;
	 stream << data.origin_service_;
	 stream << data.origin_session_serial_num_;
	 stream << data.destination_service_;
	 stream << data.destination_session_serial_num_;
	 stream << data.cur_session_serial_num_;
	 stream << data.suspend_session_serial_num_;

	 return stream;
 }