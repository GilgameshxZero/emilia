/*
Standard


*/

#pragma once

#include "utility-filesystem.h"
#include "utility-string.h"

namespace Rain {
	class Configuration {
	public:
		Configuration();
		Configuration(std::string file);

		Configuration &operator [](std::string key);
		bool has(std::string key);

		std::set<std::string> keys();
		std::string s();
		int i();
	private:
		std::map<std::string, Configuration *> children;
		std::string value;
	};

	//multiple lines, each into a separate string in the vector.
	std::vector<std::string> readMultilineFile(std::string filePath);

	//read parameters from standard parameter string, organized in lines (terminated by \n) of key:value, possibly with whitespace inbetween elements
	std::map<std::string, std::string> readParameterStream(std::stringstream &paramStream);
	std::map<std::string, std::string> readParameterString(std::string paramString);
	std::map<std::string, std::string> readParameterFile(std::string filePath);
	void writeParameterFile(std::string filePath, std::map<std::string, std::string> params);
}