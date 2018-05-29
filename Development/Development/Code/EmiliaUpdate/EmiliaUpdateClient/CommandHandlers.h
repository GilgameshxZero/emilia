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

		//helper functions for the handlers to set up a socket connection
		//returns nonzero for error
		int CHHSetupCSM(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam);

		//helper functions for the handlers which include only the code to execute the command without setting up the CSM
		//returns nonzero for error
		int CHHProdDownload(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam);
		int CHHProdStop(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam);
		int CHHProdStart(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam);
		int CHHSyncStop(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam);
		int CHHSyncStart(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam);

		//aditional helpers
		//wipes staging with prod, possibly activating CRH
		//returns 1 if need to restart program
		int CHHStageProd(CommandHandlerParam &cmhParam);
	}
}