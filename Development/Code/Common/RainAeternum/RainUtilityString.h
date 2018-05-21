/*
Standard
*/

#pragma once

#include "RainWindowsLAM.h"

#include <iomanip> //encodeURL and decodeURL
#include <sstream>
#include <string>
#include <algorithm> 
#include <cctype>

namespace Rain {
	template <typename T>
	std::string tToStr(T x) {
		std::stringstream ss;
		ss << x;
		return ss.str();
	}

	template <typename T>
	T strToT(std::string s) {
		T r;
		std::stringstream ss(s);
		ss >> r;
		return r;
	}

	void encodeBase64(const std::string &str, std::string &rtrn);
	void decodeBase64(const std::string &str, std::string &rtrn);

	std::string encodeURL(const std::string &value);
	std::string decodeURL(const std::string &value);

	//length of x, interpreted as a base10 string
	int intLogLen(int x);

	//string whitespace trim functions
	void strLTrim(std::string &s);
	void strRTrim(std::string &s);
	void strTrim(std::string &s);

	//concatenate a string to another, then a NULL character; useful for environment variable block manipulation
	std::string &appendEnvVar(std::string &envBlock, std::string newVar);

	std::string &toLowercase(std::string &s);
}