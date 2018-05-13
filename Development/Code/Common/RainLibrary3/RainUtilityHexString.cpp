#include "RainUtilityHexString.h"

namespace Rain {
	int hexDigToInt(char hex) {
		if (hex >= '0' && hex <= '9')
			return hex - '0';
		return hex - 'a' + 10;
	}

	char hexToChar(std::string hex) {
		unsigned char byte = static_cast<unsigned char>(hexDigToInt(hex[0]) * 16 + hexDigToInt(hex[1]));
		return reinterpret_cast<char &>(byte);
	}

	std::string charToHex(char c) {
		std::stringstream ss;
		ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(reinterpret_cast<unsigned char &>(c));
		return ss.str();
	}

	std::string &appHexByte(std::string &data, std::string hex) {
		data.push_back(hexToChar(hex));
		return data;
	}

	std::string &appHexBytes(std::string &data, std::string hexStr) {
		for (int a = 0; a < hexStr.length(); a += 2)
			appHexByte(data, hexStr.substr(a, 2));
		return data;
	}

	std::string byteStrToHexStr(std::string data) {
		std::stringstream ss;
		for (int a = 0; a < data.length(); a++)
			ss << charToHex(data[a]);
		return ss.str();
	}

	std::string reverseByteStr(std::string byteStr) {
		std::reverse(byteStr.begin(), byteStr.end());
		return byteStr;
	}
}