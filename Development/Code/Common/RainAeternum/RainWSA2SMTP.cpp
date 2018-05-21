#include "RainWSA2SMTP.h"

namespace Rain {
	int getSMTPStatusCode(std::string &message) {
		//get the last line
		std::size_t lastCLRF = message.rfind("\r\n"),
			stlCLRF = message.rfind("\r\n", lastCLRF - 1);
		if (lastCLRF == std::string::npos)
			return -1;
		std::string lastLine = message.substr((stlCLRF == std::string::npos ? 0 : stlCLRF) , lastCLRF);

		//status code is anything up to the first space
		std::size_t firstSpace = lastLine.find(" ");
		if (firstSpace == std::string::npos)
			return -1;
		return Rain::strToT<int>(lastLine.substr(0, firstSpace));
	}
}