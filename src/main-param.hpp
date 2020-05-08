#pragma once
#include "../rain/src/rain.hpp"

#include "http-server-param.hpp"
#include "smtp-server-param.hpp"
#include "deploy-server-param.hpp"
#include "deploy-client-param.hpp"

namespace Emilia {
	struct MainParam {
		std::string project;
		Rain::Configuration *config = NULL;

		HTTPServer::ConnectionCallerParam httpCCP;
		SMTPServer::ConnectionCallerParam smtpCCP;
		DeployServer::ConnectionCallerParam deployCCP;
		DeployClient::ConnectionHandlerParam *deployCHP = NULL;
		Rain::ServerManager httpSM, smtpSM;
		Rain::HeadedServerManager deploySM;

		Rain::LogStream *logGeneral = NULL,
			*logHTTP = NULL,
			*logSMTP = NULL,
			*logDeploy = NULL;

		//NULL, or a valid socket to a remote server after connect is issued
		Rain::HeadedClientSocketManager *remoteCSM = NULL;

		//cerr and memory leak log handles
		std::pair<std::streambuf *, std::ofstream *> cerrRedirect;
		HANDLE hFMemLeak = NULL;
	};
}