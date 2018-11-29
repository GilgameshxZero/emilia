#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "update-client-param.h"

namespace Emilia {
	struct CommandHandlerParam {
		std::map<std::string, std::string> *config = NULL;

		Rain::LogStream *logger = NULL;

		//servers
		Rain::ServerManager *httpSM = NULL, *smtpSM = NULL;

		//NULL, or a valid socket to a remote server after connect is issued
		Rain::ClientSocketManager *remoteCSM = NULL;

		UpdateClient::ConnectionHandlerParam *chParam;

		//types of files
		std::set<std::string> excAbsSet, ignAbsSet, notSharedAbsSet;
		std::vector<std::string> excVec, ignVec;
	};
}