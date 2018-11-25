#include "algorithm-libraries.h"

namespace Rain {
	unsigned long long fastModExp(unsigned long long base, unsigned long long exp, unsigned long long mod) {
		if (exp == 0)
			return 1;

		unsigned long long halfExp = fastModExp(base, exp / 2, mod);
		return (((halfExp * halfExp) % mod) * (exp % 2 ? base : 1)) % mod;
	}

	std::size_t rabinKarpMatch(const std::string &text, std::string pattern, unsigned long long prime) {
		const unsigned long long alphabetSize = 256;
		unsigned long long curHash = 0, targetHash = 0,
			basePower = fastModExp(alphabetSize, pattern.length() - 1, prime);

		//shift chars by 128 to make them signed

		//calculate the hash of the pattern
		for (std::size_t a = 0; a < pattern.length(); a++)
			targetHash = ((targetHash * alphabetSize) + (pattern[a] + 128LL)) % prime;

		for (std::size_t a = 0; a < text.length(); a++) {
			//delete the oldest character from the current hash if available
			if (a >= pattern.length())
				curHash = (prime + curHash - (basePower * (text[a - pattern.length()] + 128LL)) % prime) % prime;
			curHash = ((curHash * alphabetSize) + (text[a] + 128LL)) % prime;

			if (curHash == targetHash && text.substr(a - pattern.length() + 1, pattern.length()) == pattern) //check if string actually matches
				return a - pattern.length() + 1;
		}

		return -1;
	}
}