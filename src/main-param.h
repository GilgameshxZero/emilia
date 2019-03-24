#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "http-server.h"
#include "smtp-server.h"
#include "deploy-server-param.h"
#include "deploy-client-param.h"

namespace Emilia {
	struct MainParam {
		std::string project;
		Rain::Configuration *config;

		HTTPServer::ConnectionCallerParam httpCCP;
		SMTPServer::ConnectionCallerParam smtpCCP;
		DeployServer::ConnectionCallerParam deployCCP;
		DeployClient::ConnectionHandlerParam *deployCHP;
		Rain::ServerManager httpSM, smtpSM;
		Rain::HeadedServerManager deploySM;

		Rain::LogStream *logGeneral, *logHTTP, *logSMTP, *logDeploy;

		//NULL, or a valid socket to a remote server after connect is issued
		Rain::HeadedClientSocketManager *remoteCSM = NULL;

		//cerr and memory leak log handles
		std::pair<std::streambuf *, std::ofstream *> cerrRedirect;
		HANDLE hFMemLeak;
	};
}