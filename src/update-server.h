#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "update-server-param.h"
#include "update-helper.h"

#include <ShellAPI.h>

namespace Emilia {
	namespace UpdateServer {
		//handlers for RecvThread
		int onConnect(void *funcParam);
		int onMessage(void *funcParam);
		int onDisconnect(void *funcParam);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);

		//specific handlers for different messages
		int HRAuthenticate(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
		int HRPush(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
		int HRPushExclusive(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
		int HRPull(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
		int HRStart(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
		int HRStop(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
	}
}