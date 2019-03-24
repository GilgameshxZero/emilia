#include "config.h"

namespace Rain {
	Configuration::Configuration() {
	}
	Configuration::Configuration(std::string file) {
		std::ifstream in(file, std::ios::binary);
		std::vector<Configuration *> cstack;

		cstack.push_back(this);
		std::string line;
		while (std::getline(in, line)) {
			if (line.back() == '\r') {
				line.pop_back();
			}
			if (line.empty()) {
				continue;
			}

			//scope to right configuration option
			std::size_t level;
			for (level = 0; level < line.size(); level++) {
				if (line[level] != '\t') {
					break;
				}
			}
			while (cstack.size() - 1 > level) {
				cstack.pop_back();
			}

			//this option
			Configuration *option = new Configuration();
			line = Rain::strTrimWhite(line);
			std::size_t keyInd;
			for (keyInd = 0; keyInd < line.size(); keyInd++) {
				if (std::isspace(line[keyInd])) {
					break;
				}
			}
			cstack.back()->children.insert(std::make_pair(line.substr(0, keyInd), option));
			option->value = Rain::strTrimWhite(line.substr(keyInd));

			//in case the next line scopes into this option
			cstack.push_back(option);
		}

		in.close();
	}
	Configuration &Configuration::operator [](std::string key) {
		return *this->children[key];
	}
	bool Configuration::has(std::string key) {
		return this->children.find(key) != this->children.end();
	}

	std::set<std::string> Configuration::keys() {
		std::set<std::string> s;
		for (auto it = this->children.begin(); it != this->children.end(); it++) {
			s.insert(it->first);
		}
		return s;
	}
	std::string Configuration::s() {
		return this->value;
	}
	int Configuration::i() {
		return Rain::strToT<int>(this->value);
	}

	std::vector<std::string> readMultilineFile(std::string filePath) {
		std::ifstream fileIn;
		std::vector<std::string> ret;

		fileIn.open(filePath, std::ios::binary);
		std::stringstream ss;
		ss << fileIn.rdbuf();

		std::string value = "";
		std::getline(ss, value);

		while (value.length() != 0) {
			ret.push_back(Rain::strTrimWhite(value));
			value = "";
			std::getline(ss, value);
		}

		fileIn.close();
		return ret;
	}

	std::map<std::string, std::string> readParameterStream(std::stringstream &paramStream) {
		std::string key = "", value;
		std::map<std::string, std::string> params;
		std::getline(paramStream, key, ':');

		while (key.length() != 0) {
			std::getline(paramStream, value);
			Rain::strTrimWhite(&value);
			Rain::strTrimWhite(&key);
			params[key] = value;
			key = "";
			std::getline(paramStream, key, ':');
		}
		return params;
	}
	std::map<std::string, std::string> readParameterString(std::string paramString) {
		std::stringstream ss;
		ss << paramString;
		return readParameterStream(ss);
	}
	std::map<std::string, std::string> readParameterFile(std::string filePath) {
		std::ifstream fileIn;

		fileIn.open(filePath, std::ios::binary);
		std::stringstream ss;
		ss << fileIn.rdbuf();
		std::map<std::string, std::string> ret = readParameterStream(ss);
		fileIn.close();
		return ret;
	}
	void writeParameterFile(std::string filePath, std::map<std::string, std::string> params) {
		std::ofstream fileOut(filePath, std::ios::binary);

		for (auto it = params.begin(); it != params.end(); it++) {
			fileOut << it->first << ": " << it->second << CRLF;
		}

		fileOut.close();
	}
}