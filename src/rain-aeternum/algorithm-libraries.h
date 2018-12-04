/*
Standard
*/

#pragma once

#include "utility-filesystem.h"

#include <algorithm>
#include <fstream>
#include <string>

namespace Rain {
	//O(ln n) modded exponent
	unsigned long long fastModExp(unsigned long long base, unsigned long long exp, unsigned long long mod);

	//O(text + pattern) exact match with Rabin-Karp hashing
	//prime should be less than 2^32
	//returns -1 if not found, otherwise beginning index of the first match
	std::size_t rabinKarpMatch(const std::string &text, std::string pattern, unsigned long long prime = 1610612741);

	unsigned int checksumFileCRC32(std::string file);
}