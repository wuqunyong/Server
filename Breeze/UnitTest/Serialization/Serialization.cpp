#include "Serialization.h"

#include <iostream>

UnitTestSerialization::UnitTestSerialization(void)
{

}

UnitTestSerialization::~UnitTestSerialization(void)
{

}

int UnitTestSerialization::Run(void)
{
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

	ByteBuffer buffer;
	buffer << value_8;
	buffer << value_16;
	buffer << value_32;
	buffer << value_64;
	buffer << value_8_un;
	buffer << value_16_un;
	buffer << value_32_un;
	buffer << value_64_un;
	buffer << value_f;
	buffer << value_d;
	buffer << value_b;
	buffer << text;

	value_8 = 0;
	value_16 = 0;
	value_32 = 0;
	value_64 = 0;
	value_8_un = 0;
	value_16_un = 0;
	value_32_un = 0;
	value_64_un = 0;
	value_f = 0;
	value_d = 0;
	value_b = 0;
	text = "";

	std::cout << "length: " << buffer.Length() << std::endl;
	std::cout << (int32_t)value_8 << std::endl;
	std::cout << value_16 << std::endl;
	std::cout << value_32 << std::endl;
	std::cout << value_64 << std::endl;
	std::cout << (int32_t)value_8_un << std::endl;
	std::cout << value_16_un << std::endl;
	std::cout << value_32_un << std::endl;
	std::cout << value_64_un << std::endl;
	std::cout << value_f << std::endl;
	std::cout << value_d << std::endl;
	std::cout << value_b << std::endl;
	std::cout << text << std::endl;

	buffer >> value_8;
	buffer >> value_16;
	buffer >> value_32;
	buffer >> value_64;
	buffer >> value_8_un;
	buffer >> value_16_un;
	buffer >> value_32_un;
	buffer >> value_64_un;
	buffer >> value_f;
	buffer >> value_d;
	buffer >> value_b;
	buffer >> text;

	std::cout << "**************************" << std::endl;

	std::cout << "length: " << buffer.Length() << std::endl;
	std::cout << (int32_t)value_8 << std::endl;
	std::cout << value_16 << std::endl;
	std::cout << value_32 << std::endl;
	std::cout << value_64 << std::endl;
	std::cout << (int32_t)value_8_un << std::endl;
	std::cout << value_16_un << std::endl;
	std::cout << value_32_un << std::endl;
	std::cout << value_64_un << std::endl;
	std::cout << value_f << std::endl;
	std::cout << value_d << std::endl;
	std::cout << value_b << std::endl;
	std::cout << text << std::endl;



	buffer << value_8;
	buffer << value_16;
	buffer << value_32;
	buffer << value_64;

	buffer >> value_8;
	buffer >> value_16;

	buffer << value_8;
	buffer << value_16;
	buffer << value_32;
	buffer << value_64;

	value_8 = 0;
	value_16 = 0;
	value_32 = 0;
	value_64 = 0;

	buffer >> value_32;
	buffer >> value_64;

	buffer >> value_8;
	buffer >> value_16;
	buffer >> value_32;

	std::cout << "length: " << buffer.Length() << std::endl;
	std::cout << (int32_t)value_8 << std::endl;
	std::cout << value_16 << std::endl;
	std::cout << value_32 << std::endl;
	std::cout << value_64 << std::endl;

	return 0;
}
