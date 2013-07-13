#ifndef __SHARED_DB_MYSQL_CONNECTION_H__
#define __SHARED_DB_MYSQL_CONNECTION_H__

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <string>
#include <queue>

#include <mysql.h>

#include "ResultSet.h"

struct MySQLConnectionInfo
{
	std::string host;
	std::string user;
	std::string password;
	std::string database;
	unsigned int port;
};

class MySQLConnection
{
public:
	MySQLConnection(void);
	virtual ~MySQLConnection(void);

	void Initialize(MySQLConnectionInfo& conn_info);

	virtual bool Connect(void);
	void Disconnect(void);

public:

	bool Query(const char* sql, ResultSet* & ref_ptr_set, bool commit_flags = true);
    //把2进制字符串转换为C风格的字符串
    bool ConvertBinaryStrToCStr(const std::string& from, std::string& to);

	uint32_t GetLastError(void)
	{
		return mysql_errno(this->mysql_);
	}

	MYSQL* GetHandle(void)
	{
		return this->mysql_;
	}

private:
	bool HandleMySQLErrno(uint32_t err_no);
	bool CanTryReconnect(void);

private:
	MYSQL *               mysql_;
	MySQLConnectionInfo  connection_info_;
	uint32_t  re_connect_count_;
};

#endif