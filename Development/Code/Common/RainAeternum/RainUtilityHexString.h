/*
Standard
*/

/*
Needs brushing up. Originally used with TLS, but not very standard library.
*/

#pragma once

#include <iomanip>
#include <sstream>
#include <string>

namespace Rain {
	int hexDigToInt(char hex);
	char hexToChar(std::string hex);
	std::string charToHex(char c);
	std::string &appHexByte(std::string &data, std::string hex);
	std::string &appHexBytes(std::string &data, std::string hexStr);
	std::string byteStrToHexStr(std::string data);
	std::string reverseByteStr(std::string byteStr);

	template <typename T>
	std::string tToByteStr(T x) {
		std::string ret;
		int xLen = sizeof x;
		char *xC = reinterpret_cast<char *>(static_cast<void *>(&x));
		for (int a = 0; a < xLen; a++)
			ret.push_back(xC[a]);
		return ret;
	}
}