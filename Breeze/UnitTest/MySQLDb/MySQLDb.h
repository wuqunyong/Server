#ifndef __MYSQLDB_MYSQLDB_H__
#define __MYSQLDB_MYSQLDB_H__

#include "Db/MySQLConnection.h"

class UnitTestMySQL
{
public:
	UnitTestMySQL(void);
	~UnitTestMySQL(void);

public:
	int Run(void);

};


#endif
