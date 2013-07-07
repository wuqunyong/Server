#include "RPCMySQL.h"

#include <string>
#include <sstream>

#include "Net/Fd.h"
#include "Utils/CommandBase.h"

#include "Db/ResultSet.h"
#include "Net/Ctx.h"
#include "Net/MySQLRPCClient.h"
#include "Net/MySQLRPCServer.h"
#include "Net/IOThread.h"


UnitTestRPCMysql::UnitTestRPCMysql(void)
{

}

UnitTestRPCMysql::~UnitTestRPCMysql(void)
{

}

int UnitTestRPCMysql::Run(void)
{
	FREEZE_NET::Ctx context(2);

	MySQLConnectionInfo conn_info;
	conn_info.host = "127.0.0.1";
	conn_info.user = "root";
	conn_info.password = "123456";
	conn_info.database = "test";
	conn_info.port = 3306;
	context.MysqlRPCServer()->Connect(conn_info);

	std::stringstream ss;
	ss << "DROP TABLE IF EXISTS `tb_role_farm`;";

	std::string table;
	std::string query_statement = ss.str();
	CommandBaseEx* callback_cmd = NULL;
	ResultSet* query_result = NULL;

	context.mysql_rpc_client_->SendRPCNotify(query_statement.c_str());

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

	query_statement = ss.str();

	context.mysql_rpc_client_->SendRPCNotify(query_statement.c_str());

	int loop_count = 100;

	while(loop_count--)
	{
		ss.str("");

		ss << "INSERT INTO `tb_role_farm` VALUES (NULL, '100004290', '1', '1', '1', 'qwert123'), (NULL, '200004290', '3', '3', '3', '333333qwert123');";

		callback_cmd = NULL;
		query_statement = ss.str();
		context.mysql_rpc_client_->SendRPCNotify(query_statement.c_str());

		ss.str("");

		ss << "UPDATE `tb_role_farm` SET `farm_level`='2',`farm_cur_experience`='2',`today_add_experience`='2' WHERE (`id`='1') ";

		callback_cmd = NULL;
		query_statement = ss.str();
		context.mysql_rpc_client_->SendRPCNotify(query_statement.c_str());

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

		using namespace FREEZE_NET;
		callback_cmd = NULL;
		callback_cmd = new MethodClosureEx0<MySQLRPCClient> (context.mysql_rpc_client_, &MySQLRPCClient::HandlePRCResponse, true);

		//callback_cmd = new CommandBaseEx();
		query_statement = ss.str();
		context.mysql_rpc_client_->SendRPC(query_statement.c_str(), callback_cmd, 0);
	}

	std::cout << "Enter any key to eixt !" << std::endl;
	std::string end;
	std::cin >> end;

	return 0;
}
