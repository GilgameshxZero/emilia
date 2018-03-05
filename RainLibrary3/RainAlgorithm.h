/*
Standard
*/

#pragma once

#include <string>

namespace Rain {
	//O(ln n) modded exponent
	unsigned long long fastModExp(unsigned long long base, unsigned long long exp, unsigned long long mod);

	//O(1) exact match with Rabin-Karp hashing
	//prime should be less than 2^32
	//returns -1 if not found, otherwise beginning index of the first match
	std::size_t rabinKarpMatch(const std::string &text, std::string pattern, unsigned long long prime = 1610612741);
}