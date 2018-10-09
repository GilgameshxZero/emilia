/*
Standard
*/

/*
Compatiable with all OS.
*/

#pragma once

#include "windows-lam-include.h"

#include <iomanip> //encodeURL and decodeURL
#include <sstream>
#include <string>
#include <algorithm> 
#include <cctype>
#include <vector>

namespace Rain {
	//converts from any type to string using stringstream
	template <typename T>
	std::string tToStr(T x) {
		std::stringstream ss;
		ss << x;
		return ss.str();
	}
	//converts from a string to some other type using stringstream
	template <typename T>
	T strToT(std::string s) {
		T r;
		std::stringstream ss(s);
		ss >> r;
		return r;
	}

	//converts to/from unicode/multibyte
	std::wstring mbStrToWStr(std::string path);
	std::string wStrToMBStr(std::wstring s);

	//convert string to lowercase
	std::string *strToLower(std::string *s);
	std::string strToLower(std::string s);

	//trim whitespace from front and end of string
	std::string *strTrimWhite(std::string *s);
	std::string strTrimWhite(std::string s);

	//encodes and decodes Base-64 format
	//pointer versions return normal strings, since none of them modify parameters
	char intEncodeB64(int x);
	int chrDecodeB64(char c);
	std::string strEncodeB64(const std::string *str);
	std::string strEncodeB64(std::string str);
	std::string strDecodeB64(const std::string *str);
	std::string strDecodeB64(std::string str);

	//encode and decode from URL format
	//does not modify parameters if passed by pointer
	//encodeURL taken from http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
	//decodeURL taken from http://stackoverflow.com/questions/2673207/c-c-url-decode-library
	std::string strEncodeURL(const std::string *value);
	std::string strEncodeURL(std::string value);
	std::string strDecodeURL(const std::string *value);
	std::string strDecodeURL(std::string value);

	//transform b16 integer to b10 integer
	int b16ToB10(char hex);
	
	//convert two hex digits to a char and back
	char hexToChr(std::pair<char, char> hex);
	std::pair<char, char> chrToHex(char c);

	//push a single character to a string, where character is specified as a hex pair
	std::string *strPushHexChr(std::string *data, std::pair<char, char> hex);
	std::string strPushHexChr(std::string data, std::pair<char, char> hex);

	//push a hex string to a string, converting the hex string to a character string first
	std::string *strPushHexStr(std::string *data, std::string *hexStr);
	std::string *strPushHexStr(std::string *data, std::string hexStr);
	std::string strPushHexStr(std::string data, std::string *hexStr);
	std::string strPushHexStr(std::string data, std::string hexStr);

	//converts from byte string to hex string and back
	std::string strEncodeToHex(std::string *data);
	std::string strEncodeToHex(std::string data);
	std::string strDecodeToHex(std::string *data);
	std::string strDecodeToHex(std::string data);

	//converts any type to a byte string
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