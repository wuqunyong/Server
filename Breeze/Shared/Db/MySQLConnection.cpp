#include "MySQLConnection.h"

MySQLConnection::MySQLConnection(void)
{
	this->re_connect_count_ = 0;
}

MySQLConnection::~MySQLConnection(void)
{
	this->Disconnect();
}

void MySQLConnection::Initialize(MySQLConnectionInfo& conn_info)
{
	this->connection_info_ = conn_info;
}

bool MySQLConnection::Connect(void)
{
	MYSQL *mysql_con;
	mysql_con = mysql_init(NULL);
	if (!mysql_con)
	{
		std::cout << "Could not initialize Mysql connection to database " << this->connection_info_.database << std::endl;
		return false;
	}

	this->mysql_ = mysql_real_connect(mysql_con, this->connection_info_.host.c_str(),
		this->connection_info_.user.c_str(), this->connection_info_.password.c_str(),
		this->connection_info_.database.c_str(), this->connection_info_.port, NULL, CLIENT_MULTI_RESULTS);

	if (this->mysql_)
	{
		my_bool my_true = true;
		mysql_autocommit(this->mysql_, my_true);

		return true;
	}
	else
	{
		std::cout << "sql: Connection failed. Reason was " << mysql_error(this->mysql_) << std::endl;

		mysql_close(mysql_con);
		return false;
	}
}

void MySQLConnection::Disconnect(void)
{
	if (NULL != this->mysql_)
	{
		mysql_close(this->mysql_);
		this->mysql_ = NULL;
	}
}

bool MySQLConnection::Query(const char* sql, ResultSet* & ref_ptr_set, bool commit_flags)
{
	if (NULL == this->mysql_ || NULL == sql)
	{
		return false;
	}
	else
	{
		//如果查询成功，返回0。如果出现错误，返回非0值
		if (mysql_query(this->mysql_, sql))
		{
			uint32_t last_errno = mysql_errno(this->mysql_);

			if (HandleMySQLErrno(last_errno))  // If it returns true, an error was handled successfully (i.e. reconnection)
			{
				return Query(sql, ref_ptr_set, commit_flags);             // Try again
			}

			return false;
		}
		else
		{
			MYSQL_RES* ptr_mysql_res = mysql_store_result(this->mysql_);

			if(ptr_mysql_res != NULL)
			{
				my_ulonglong num_rows = mysql_num_rows(ptr_mysql_res);
				if( num_rows > 0x00 )
				{
					ref_ptr_set = new ResultSet(ptr_mysql_res);
				}
				else
				{
					if(ptr_mysql_res != NULL)
					{
						mysql_free_result(ptr_mysql_res);
					}
				}

				if(commit_flags)
				{
					mysql_commit(this->mysql_);
				}
			}
			else // mysql_store_result() returned nothing; should it have?
			{
				uint32_t query_field_count = mysql_field_count(this->mysql_);
				if(0 == query_field_count)
				{
					// query does not return data
					// (it was not a SELECT)
					my_ulonglong affected_rows = mysql_affected_rows(this->mysql_);
					//std::cout << "affected_rows : " << affected_rows << std::endl;
				}
				else // mysql_store_result() should have returned data
				{
					std::cout << "Error: " << mysql_error(this->mysql_) << std::endl;
					return false;
				}
			}
		}
	}

	return true;
}

bool MySQLConnection::HandleMySQLErrno(uint32_t err_no)
{
	this->re_connect_count_++;

	if (!this->CanTryReconnect())
	{
		return false;
	}

	switch (err_no)
	{
	case 2006:  // "MySQL server has gone away"
	case 2008:  // Client ran out of memory
	case 2013:  // "Lost connection to MySQL server during query"
	case 2048:  // "Invalid connection handle"
	case 2055:  // "Lost connection to MySQL server at '%s', system error: %d"
		{
			this->Disconnect();
			if (this->Connect())
			{
				this->re_connect_count_ = 0;
				return true;
			}

			uint32_t last_errno = mysql_errno(GetHandle());   // It's possible this attempted reconnect throws 2006 at us. To prevent crazy recursive calls, sleep here.

#ifdef WIN32
			Sleep(3000);
#else
			sleep(3);                                         // Sleep 3 seconds
#endif
			return HandleMySQLErrno(last_errno);              // Call self (recursive)
		}

	default:
		std::cout << "Unhandled MySQL errno " << err_no << " Unexpected behaviour possible." << std::endl;
		return false;
	}
}

bool MySQLConnection::CanTryReconnect(void)
{
	if (this->re_connect_count_ > 10)
	{
		return false;
	}
	else
	{
		return true;
	}
}