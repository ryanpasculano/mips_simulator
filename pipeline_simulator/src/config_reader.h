/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __CONFIG_READER_H__
#define __CONFIG_READER_H__

#include <iostream>

#include "json/json.h"
#include <map>
#include <vector>
#include <string>
#include <sstream>

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*          DO NOT MODIFY THIS FILE!                            */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

class ConfigReader {
private:
	Json::Value root;
	Json::CharReaderBuilder readerBuilder;
	Json::CharReader* reader;
public:
	ConfigReader();
	virtual ~ConfigReader();
	void setJson(std::string &str);
	Json::Value getValue(std::string key);
};

#endif
