/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */


#include "config_reader.h"
#include <chrono>

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*          DO NOT MODIFY THIS FILE!                            */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

ConfigReader::ConfigReader() {
	// TODO Auto-generated constructor stub
	reader = nullptr;
}

ConfigReader::~ConfigReader() {
	// TODO Auto-generated destructor stub
}

void ConfigReader::setJson(std::string &str)
{
	reader = readerBuilder.newCharReader();
	std::string errors;
	bool success = reader->parse(str.c_str(), str.c_str() + str.size(), &root, &errors);
	delete reader;

	if(!success) {
		std::cerr << "Json failed : \n" << errors << "\n";
	}
}

Json::Value ConfigReader::getValue(std::string key) {
	return root[key];
}
