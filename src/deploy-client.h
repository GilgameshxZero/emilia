#pragma once
#include "../rain/cpp/rain-libraries.hpp"

#include "utils.h"
#include "deploy-client-param.h"

namespace Emilia {
	namespace DeployClient {
		int onConnect(void *funcParam);
		int onDisconnect(void *funcParam);
		int onMessage(void *funcParam);

		//specific handlers for different responses to requests sent by the client
		int authenticate(Rain::ClientSocketManager::DelegateHandlerParam &csmdhp);
		int sync(Rain::ClientSocketManager::DelegateHandlerParam &csmdhp);
		int server(Rain::ClientSocketManager::DelegateHandlerParam &csmdhp);
	}
}