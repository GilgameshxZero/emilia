#pragma once
#include "rain-aeternum/rain-libraries.h"

#include "deploy-server-param.h"

namespace Emilia {
	namespace DeployClient {
		typedef int(*RequestMethodHandler)(Rain::ClientSocketManager::DelegateHandlerParam &);

		struct ConnectionHandlerParam {
			std::string project;
			Rain::Configuration *config;
			Rain::LogStream *logDeploy;
			Rain::ServerManager *httpSM, *smtpSM;
			DeployServer::SyncInfo si;

			//parsed first section of the request
			std::string requestMethod;

			//password for authentication on reconnect
			std::string authPass;

			//condition variable triggered once on each authentication response
			Rain::ConditionVariable authCV;

			//triggered when 'sync'-like command is complete
			Rain::ConditionVariable connectedCommandCV;

			//whether to notify_one with authCV on disconnect, usually because server rejected connection
			bool notifyCVOnDisconnect;
		};
	}
}