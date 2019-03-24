#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "deploy-client-param.h"
#include "project-utils.h"

namespace Emilia {
	namespace DeployClient {
		int onConnect(void *funcParam);
		int onDisconnect(void *funcParam);
		int onMessage(void *funcParam);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);

		//specific handlers for different responses to requests sent by the client
		int Authenticate(Rain::ClientSocketManager::DelegateHandlerParam &csmdhp);
		int Sync(Rain::ClientSocketManager::DelegateHandlerParam &csmdhp);
		int Server(Rain::ClientSocketManager::DelegateHandlerParam &csmdhp);
	}
}