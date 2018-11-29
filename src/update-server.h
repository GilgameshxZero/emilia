#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "update-server-param.h"

#include <ShellAPI.h>

namespace Emilia {
	namespace UpdateServer {
		//handlers for RecvThread
		int onConnect(void *funcParam);
		int onMessage(void *funcParam);
		int onDisconnect(void *funcParam);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);

		//specific handlers for different messages
		int HRAuthenticate(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRPush(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRPushExclusive(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRPull(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRSync(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRStart(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRStop(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
	}
}