#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "update-server-param.h"
#include "update-client-param.h"

namespace Emilia {
	namespace UpdateHelpers {
		int ServerPushProc(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int ClientPushProc(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam);
	}
}