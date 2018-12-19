#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "update-client-param.h"
#include "update-helper.h"

namespace Emilia {
	namespace UpdateClient {
		//handlers for ClientSocketManager
		int onConnect(void *funcParam);
		int onDisconnect(void *funcParam);
		int onMessage(void *funcParam);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);

		//specific handlers for different responses to requests sent by the client
		int HRAuthenticate(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int HRPush(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int HRPushExclusive(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int HRPull(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int HRSync(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int HRStart(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int HRStop(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
	}
}