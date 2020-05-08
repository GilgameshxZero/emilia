#pragma once
#include "../rain/src/rain.hpp"

#include "utils.hpp"
#include "deploy-client-param.hpp"

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