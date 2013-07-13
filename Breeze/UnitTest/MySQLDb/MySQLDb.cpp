#include "MySQLDb.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>

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
	conn_info.password = "";
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

	uint32_t repeat_times = 10;
	while(repeat_times--)
	{
		std::ifstream is("test_image.png", std::ifstream::binary);
		if (is) 
		{
			is.seekg (0, is.end);
			int length = is.tellg();
			is.seekg (0, is.beg);

			char * buffer = new char [length];

			std::cout << "Reading " << length << " characters... ";
			// read data as a block:
			is.read (buffer,length);

			if (is)
				std::cout << "all characters read successfully.";
			else
				std::cout << "error: only " << is.gcount() << " could be read";
			is.close();

			// ...buffer contains the entire file...

			int8_t value_8 = -8;
			int16_t value_16 = -16;
			int32_t value_32 = -32;
			int64_t value_64 = -64;

			uint8_t value_8_un = 8;
			uint16_t value_16_un = 16;
			uint32_t value_32_un = 32;
			uint64_t value_64_un = 64;

			float value_f = 1.2345;
			double value_d = 6.7890;
			bool value_b = true;
			std::string text = "hello world!";


			std::string str_image;
			bool convert_result = mysql_session.ConvertBinaryStrToCStr(std::string(buffer, length), str_image);

			std::stringstream ss;
			ss << "INSERT INTO `tb_image` (`id`,`name`,`image`,`info_describe`) VALUES (NULL,'hello', '" << str_image << "','hello world!')";

			ResultSet *ptr_result = NULL;
			mysql_session.Query(ss.str().c_str(), ptr_result);
			if (NULL != ptr_result)
			{
				delete ptr_result;
				ptr_result = NULL;
			}

			delete[] buffer;
			//delete[] chunk;
		}





		std::stringstream ss;

		ss << "SELECT "
			<< "tb_image.id, "
			<< "tb_image.name, "
			<< "tb_image.image, "
			<< "tb_image.info_describe "
			<< "FROM "
			<< "tb_image ";

		uint32_t field_id = 0;
		std::string field_name;
		std::string field_image;
		std::string info_describe;

		ResultSet *record_set = NULL;
		mysql_session.Query(ss.str().c_str(), record_set);
		if (NULL != record_set)
		{
			while( record_set->MoveNext() )
			{
				if( (*record_set >> field_id)
					&& (*record_set >> field_name)
					&& (record_set->ExtractBLOB(field_image))
					&& (*record_set >> info_describe))
				{
					std::cout << " field_id : " << field_id << std::endl;
					std::cout << " field_name : " << field_name << std::endl;
					std::cout << " info_describe : " << info_describe << std::endl;
					std::cout << "***************************" << std::endl;

					std::stringstream file_ss;
					file_ss << field_name
						<< field_id
						<< ".png";

					std::string file_name = file_ss.str();
					std::ofstream outfile(file_name.c_str(),std::ofstream::binary);
					if (outfile) 
					{

						uint32_t write_size = field_image.size();

						// write to outfile
						outfile.write(field_image.c_str(), write_size);
						outfile.close();
					}
				}
			}
			delete record_set;
			record_set = NULL;


			std::cout << "finished !" << std::endl;
		}

		Sleep(5000);
	}
	return 0;
}
