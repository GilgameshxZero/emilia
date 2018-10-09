#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

#include <map>
#include <string>

namespace Monochrome3 {
	namespace EmiliaMailServer {
		struct CommandHandlerParam {
			std::map<std::string, std::string> *config;

			Rain::LogStream *logger;
		};

		typedef int(*CommandMethodHandler)(CommandHandlerParam &);

		//handlers should return nonzero to immediately terminate program
		int CHExit(CommandHandlerParam &cmhParam);
		int CHHelp(CommandHandlerParam &cmhParam);
	}
}