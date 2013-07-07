#ifndef __SHARED_DB_RESULT_SET_H__
#define __SHARED_DB_RESULT_SET_H__

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <stdint.h>
#include <stddef.h>

#include <string>
#include <sstream>
#include <iostream>

#include <mysql.h>


class ResultSet
{
	ResultSet(const ResultSet& rhs);
	ResultSet& operator=(const ResultSet& rhs);

public:
	ResultSet(MYSQL_RES* ptr_mysql_res);
	virtual ~ResultSet();

	bool MoveNext();

	bool operator>> (int8_t& ref_value);
	bool operator>> (int16_t& ref_value);
	bool operator>> (int32_t& ref_value);
	bool operator>> (int64_t& ref_value);

	bool operator>> (uint8_t& ref_value);
	bool operator>> (uint16_t& ref_value);
	bool operator>> (uint32_t& ref_value);
	bool operator>> (uint64_t& ref_value);

	bool operator>> (std::string& ref_value)
	{
		if ( (NULL == this->mysql_rows_) || (this->index_ >= this->num_fields_) )
		{
			return false;
		}

		const char* ptr_field = this->mysql_rows_[this->index_++];
		if (NULL == ptr_field)
		{
			return false;
		}

		ref_value = ptr_field;

		return true;
	}

private:
	template <typename T>
	bool Field(T& ref_value)
	{
		if ( (NULL == this->mysql_rows_) || (this->index_ >= this->num_fields_) )
		{
			return false;
		}

		const char* ptr_field = this->mysql_rows_[this->index_++];
		if (NULL == ptr_field)
		{
			return false;
		}

		std::stringstream strbuf;
		strbuf << ptr_field << std::endl;
		strbuf.flush();

		T new_value;
		strbuf >> new_value;

		ref_value = new_value;

		return true;
	};

protected:
	uint32_t   index_;
	uint32_t   num_fields_;
	MYSQL_ROW  mysql_rows_;
	MYSQL_RES* ptr_mysql_res_;
};

#endif
