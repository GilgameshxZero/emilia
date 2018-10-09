#pragma once

#include "rain-aeternum/rain-libraries.h"

#include <map>
#include <string>

namespace Emilia {
	struct CommandHandlerParam {
		std::map<std::string, std::string> *config;

		Rain::LogStream *logger;
	};

	typedef int(*CommandMethodHandler)(CommandHandlerParam &);

	//handlers should return nonzero to immediately terminate program
	int CHExit(CommandHandlerParam &cmhParam);
	int CHHelp(CommandHandlerParam &cmhParam);
}