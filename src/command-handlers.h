#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "main.h"
#include "update-client.h"

#include <map>
#include <ShellAPI.h>
#include <string>

namespace Emilia {
	struct CommandHandlerParam {
		std::map<std::string, std::string> *config = NULL;

		Rain::LogStream *logger = NULL;

		//servers
		Rain::ServerManager *httpSM = NULL, *smtpSM = NULL;

		//NULL, or a valid socket to a remote server after connect is issued
		Rain::ClientSocketManager *remoteCSM = NULL;

		UpdateClient::ConnectionHandlerParam *chParam;
	};

	typedef int(*CommandMethodHandler)(CommandHandlerParam &);

	//handlers should return nonzero to immediately terminate program
	int CHExit(CommandHandlerParam &cmhParam);
	int CHHelp(CommandHandlerParam &cmhParam);
	int CHConnect(CommandHandlerParam &cmhParam);
	int CHDisconnect(CommandHandlerParam &cmhParam);
	int CHPush(CommandHandlerParam &cmhParam);
	int CHPushExclusive(CommandHandlerParam &cmhParam);
	int CHPull(CommandHandlerParam &cmhParam);
	int CHSync(CommandHandlerParam &cmhParam);
	int CHStart(CommandHandlerParam &cmhParam);
	int CHStop(CommandHandlerParam &cmhParam);
	int CHRestart(CommandHandlerParam &cmhParam);

	//helper functions for the handlers to set up a socket connection
	//returns nonzero for error
	int CHHSetupCSM(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam);

	//helper functions for the handlers which include only the code to execute the command without setting up the CSM
	//returns nonzero for error
	int CHHProdDownload(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam);
	int CHHProdStop(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam);
	int CHHProdStart(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam);
	int CHHSyncStop(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam);
	int CHHSyncStart(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam);

	//additional helpers
	//wipes staging with prod, possibly activating CRH
	//returns 1 if need to restart program
	//pass a restartCode to pass as command line to the restarted program
	int CHHStageProd(CommandHandlerParam &cmhParam, std::string restartCode);
}