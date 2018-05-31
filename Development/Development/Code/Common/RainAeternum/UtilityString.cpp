#include "UtilityString.h"

namespace Rain {
	std::wstring mbStrToWStr(std::string s) {
		static wchar_t *buffer;
		static int bytes;
		static std::wstring ret;
		
		buffer = new wchar_t[s.length()];
		bytes = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.length()), buffer, static_cast<int>(s.length()));
		ret = std::wstring(buffer, bytes);
		delete[] buffer;
		return ret;
	}
	std::string wStrToMBStr(std::wstring s) {
		static char *buffer;
		static int bytes;
		static std::string ret;

		buffer = new char[s.length() * 4];
		bytes = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), static_cast<int>(s.length()), buffer, static_cast<int>(s.length() * 4), NULL, NULL);
		ret = std::string(buffer, bytes);
		delete[] buffer;
		return ret;
	}

	std::string *strToLower(std::string *s) {
		for (std::size_t a = 0; a < s->length(); a++)
			(*s)[a] = tolower((*s)[a]);
		return s;
	}
	std::string strToLower(std::string s) {
		return *strToLower(&s);
	}

	std::string *strTrimWhite(std::string *s) {
		//trim left
		s->erase(s->begin(), std::find_if(s->begin(), s->end(), [](int ch) {
			return !std::isspace(ch);
		}));
		//trim right
		s->erase(std::find_if(s->rbegin(), s->rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s->end());
		return s;
	}
	std::string strTrimWhite(std::string s) {
		return *strTrimWhite(&s);
	}

	char chrEncodeB64(char c) {
		if (c == '/')
			return 63;
		if (c == '+')
			return 62;
		if (c < 58)
			return c - '0' + 52;
		if (c < 91)
			return c - 'A';
		else //if (c < 123)
			return c - 'a' + 26;
	}
	char chrDecodeB64(char x) {
		if (x < 26)
			return x + 'A';
		if (x < 52)
			return x - 26 + 'a';
		if (x < 62)
			return x - 52 + '0';
		if (x == 62)
			return '+';
		else //if (x == 63)
			return '/';
	}
	std::string strEncodeB64(const std::string *str) {
		static const int MAXBITS = 6;
		static const unsigned char MASK = 0xFF;

		std::string ret;
		int bitsInChr = 0;
		int curChr = 0;
		for (std::size_t a = 0; a < str->length(); a++) {
			curChr |= (((*str)[a] & (MASK << (bitsInChr + 2))) >> (bitsInChr + 2));
			ret.push_back(chrEncodeB64(static_cast<char>(static_cast<int>(curChr))));
			curChr = ((*str)[a] & (MASK >> (MAXBITS - bitsInChr)));
			bitsInChr = bitsInChr + 2;
			curChr <<= MAXBITS - bitsInChr;

			if (bitsInChr == MAXBITS) {
				ret.push_back(chrEncodeB64(chrEncodeB64(static_cast<char>(static_cast<int>(curChr)))));
				curChr = 0;
				bitsInChr = 0;
			}
		}

		if (bitsInChr != 0)
			ret.push_back(chrEncodeB64(chrEncodeB64(static_cast<char>(static_cast<int>(curChr)))));
		return ret;
	}
	std::string strEncodeB64(std::string str) {
		return strEncodeB64(&str);
	}
	std::string strDecodeB64(const std::string *str) {
		static const int MAXBITS = 8;
		static const unsigned char MASK = 0xFF;

		std::string ret;
		int bitsInChr = 0;
		char curChr = 0, curint;
		for (std::size_t a = 0; a < str->length(); a++) {
			curint = chrDecodeB64((*str)[a]);
			if (bitsInChr == 0) {
				curChr = (curint << 2);
				bitsInChr = 6;
				continue;
			}
			curChr |= (((curint << 2) & (MASK << bitsInChr)) >> bitsInChr);
			ret.push_back(curChr);
			curChr = (curint & (MASK >> (10 - bitsInChr)));
			bitsInChr = bitsInChr - 2;
			curChr <<= MAXBITS - bitsInChr;
		}

		return ret;
	}
	std::string strDecodeB64(std::string str) {
		return strDecodeB64(&str);
	}

	std::string strEncodeURL(const std::string *value) {
		std::ostringstream escaped;
		escaped.fill('0');
		escaped << std::hex;

		for (std::string::const_iterator i = value->begin(), n = value->end(); i != n; ++i) {
			std::string::value_type c = (*i);

			// Keep alphanumeric and other accepted characters intact
			if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
				escaped << c;
				continue;
			}

			// Any other characters are percent-encoded
			escaped << std::uppercase;
			escaped << '%' << std::setw(2) << int((unsigned char) c);
			escaped << std::nouppercase;
		}

		return escaped.str();
	}
	std::string strEncodeURL(std::string value) {
		return strEncodeURL(&value);
	}
	std::string strDecodeURL(const std::string *value) {
		std::string rtrn;
		char a, b;
		const char *src = value->c_str();
		while (*src) {
			if ((*src == '%') &&
				((a = src[1]) && (b = src[2])) &&
				(isxdigit(a) && isxdigit(b))) {
				if (a >= 'a')
					a -= 'a' - 'A';
				if (a >= 'A')
					a -= ('A' - 10);
				else
					a -= '0';
				if (b >= 'a')
					b -= 'a' - 'A';
				if (b >= 'A')
					b -= ('A' - 10);
				else
					b -= '0';
				rtrn += 16 * a + b;
				src += 3;
			} else if (*src == '+') {
				rtrn += ' ';
				src++;
			} else {
				rtrn += *src++;
			}
		}

		return rtrn;
	}
	std::string strDecodeURL(std::string value) {
		return strDecodeURL(&value);
	}

	int b16ToB10(char hex) {
		if (hex >= '0' && hex <= '9')
			return hex - '0';
		return static_cast<int>(hex - 'a' + 10);
	}

	char hexToChr(std::pair<char, char> hex) {
		unsigned char byte = static_cast<unsigned char>(b16ToB10(hex.first) * 16 + b16ToB10(hex.second));
		return reinterpret_cast<char &>(byte);
	}
	std::pair<char, char> chrToHex(char c) {
		std::stringstream ss;
		ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(reinterpret_cast<unsigned char &>(c));
		return std::make_pair(ss.str()[0], ss.str()[1]);
	}

	std::string *strPushHexChr(std::string *data, std::pair<char, char> hex) {
		data->push_back(hexToChr(hex));
		return data;
	}
	std::string strPushHexChr(std::string data, std::pair<char, char> hex) {
		return *strPushHexChr(&data, hex);
	}

	std::string *strPushHexStr(std::string *data, std::string *hexStr) {
		for (std::size_t a = 0; a < hexStr->length(); a += 2)
			strPushHexChr(data, std::make_pair((*hexStr)[a], (*hexStr)[a + 1]));
		return data;
	}
	std::string *strPushHexStr(std::string *data, std::string hexStr) {
		return strPushHexStr(data, &hexStr);
	}
	std::string strPushHexStr(std::string data, std::string *hexStr) {
		return *strPushHexStr(&data, hexStr);
	}
	std::string strPushHexStr(std::string data, std::string hexStr) {
		return *strPushHexStr(&data, hexStr);
	}

	std::string strEncodeToHex(std::string *data) {
		return strPushHexStr("", data);
	}
	std::string strEncodeToHex(std::string data) {
		return strEncodeToHex(&data);
	}
	std::string strDecodeToHex(std::string *data) {
		std::stringstream ss;
		for (std::size_t a = 0; a < data->length(); a++) {
			static std::pair<char, char> conv;
			conv = chrToHex((*data)[a]);
			ss << conv.first << conv.second;
		}
		return ss.str();
	}
	std::string strDecodeToHex(std::string data) {
		return strDecodeToHex(&data);
	}
}