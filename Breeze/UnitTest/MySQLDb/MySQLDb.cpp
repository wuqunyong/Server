#include "MySQLDb.h"

UnitTestMySQL::UnitTestMySQL(void)
{

}

UnitTestMySQL::~UnitTestMySQL(void)
{

}

int UnitTestMySQL::Run(void)
{
	MySQLConnection mysql_session;

	MySQLConnectionInfo conn_info;
	conn_info.host = "127.0.0.1";
	conn_info.user = "root";
	conn_info.password = "123456";
	conn_info.database = "test";
	conn_info.port = 3306;

	mysql_session.Initialize(conn_info);

	bool result = mysql_session.Connect();
	if (!result)
	{
		std::cout << "mysql connect error !" << std::endl;
	}

	std::stringstream ss;
	ss << "DROP TABLE IF EXISTS `tb_role_farm`;";

	ResultSet* record_set = NULL;
	result = mysql_session.Query(ss.str().c_str(), record_set);
	if (NULL != record_set)
	{
		delete record_set;
		record_set = NULL;
	}

	ss.str("");

	ss << "CREATE TABLE `tb_role_farm` ( "
		<< "`id` int(11) NOT NULL AUTO_INCREMENT, "
		<< "`role_id` int(11) NOT NULL DEFAULT '0', "
		<< "`farm_level` int(11) NOT NULL DEFAULT '1', "
		<< "`farm_cur_experience` int(11) NOT NULL DEFAULT '0', "
		<< "`today_add_experience` int(11) NOT NULL DEFAULT '0', "
		<< "`str_value` varchar(1024) NOT NULL DEFAULT '', "
		<< "PRIMARY KEY (`id`) "
		<< ") ENGINE=InnoDB AUTO_INCREMENT=323 DEFAULT CHARSET=utf8; ";

	result = mysql_session.Query(ss.str().c_str(), record_set);
	if (NULL != record_set)
	{
		delete record_set;
		record_set = NULL;
	}

	ss.str("");

	ss << "INSERT INTO `tb_role_farm` VALUES ('1', '100004290', '1', '1', '1', 'qwert123'), ('2', '200004290', '3', '3', '3', '333333qwert123');";
	result = mysql_session.Query(ss.str().c_str(), record_set);
	if (NULL != record_set)
	{
		delete record_set;
		record_set = NULL;
	}

	ss.str("");

	ss << "UPDATE `tb_role_farm` SET `farm_level`='2',`farm_cur_experience`='2',`today_add_experience`='2' WHERE (`id`='1') ";
	result = mysql_session.Query(ss.str().c_str(), record_set);
	if (NULL != record_set)
	{
		delete record_set;
		record_set = NULL;
	}
	ss.str("");

	ss << "SELECT "
		<< "tb_role_farm.id, "
		<< "tb_role_farm.role_id, "
		<< "tb_role_farm.farm_level, "
		<< "tb_role_farm.farm_cur_experience, "
		<< "tb_role_farm.today_add_experience, "
		<< "tb_role_farm.str_value "
		<< "FROM "
		<< "tb_role_farm ";

	result = mysql_session.Query(ss.str().c_str(), record_set);
	if (NULL != record_set)
	{
		while( record_set->MoveNext() )
		{
			uint32_t field_1 = 0;
			uint32_t field_2 = 0;
			uint32_t field_3= 0;
			uint32_t field_4 = 0;
			uint32_t field_5 = 0;
			std::string field_6;

			if( (*record_set >> field_1)
				&& (*record_set >> field_2)
				&& (*record_set >> field_3)
				&& (*record_set >> field_4)
				&& (*record_set >> field_5)
				&& (*record_set >> field_6))
			{
				std::cout << "field_1 : " << field_1 << std::endl;
				std::cout << "field_2 : " << field_2 << std::endl;
				std::cout << "field_3 : " << field_3 << std::endl;
				std::cout << "field_4 : " << field_4 << std::endl;
				std::cout << "field_5 : " << field_5 << std::endl;
				std::cout << "field_6 : " << field_6 << std::endl;

			}
		}

		delete record_set;
		record_set = NULL;
	}

	return 0;
}
