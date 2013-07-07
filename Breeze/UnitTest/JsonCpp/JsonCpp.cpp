#include "JsonCpp.h"

#include <fstream>

#include <json/json.h>
using namespace Json; 

UnitTestJsonCpp::UnitTestJsonCpp(void)
{
	//no operate
}

UnitTestJsonCpp::~UnitTestJsonCpp(void)
{
	//no operate
}

int UnitTestJsonCpp::Run(void)
{
	Json::Value root;
	Json::Value arrayObj;
	Json::Value item;

	for (int i = 0; i < 10; i ++)
	{
		item["key"] = i;
		arrayObj.append(item);
	}

	root["key1"] = "value1";
	root["key2"] = "value2";
	root["array"] = arrayObj;
	std::string out = root.toStyledString();
	std::cout << out << std::endl;


	Json::StyledWriter writer;   
	std::string output = writer.write(root);

	std::ofstream out_file("duplicate_config.json" );   
	out_file << output;
	out_file.flush();

	return 0;
}