#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

#include "ConnectionHandlers.h"

#include <map>
#include <shellapi.h>
#include <string>

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		struct CommandHandlerParam {
			std::map<std::string, std::string> *config;

			Rain::LogStream *logger;
		};

		typedef int(*CommandMethodHandler)(CommandHandlerParam &);

		//handlers should return nonzero to immediately terminate program
		int CHHelp(CommandHandlerParam &cmhParam);
		int CHStageDev(CommandHandlerParam &cmhParam);
		int CHDeployStaging(CommandHandlerParam &cmhParam);
		int CHProdDownload(CommandHandlerParam &cmhParam);
		int CHStageProd(CommandHandlerParam &cmhParam);
		int CHProdStop(CommandHandlerParam &cmhParam);
		int CHProdStart(CommandHandlerParam &cmhParam);
		int CHSyncStop(CommandHandlerParam &cmhParam);
		int CHSyncStart(CommandHandlerParam &cmhParam);
	}
}