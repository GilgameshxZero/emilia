#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "update-server-param.h"
#include "update-client-param.h"

#include <ShellAPI.h>

namespace Emilia {
	namespace UpdateHelper {
		int ServerPushProc(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam, std::string method);
		int ClientPushProc(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam, std::string method);
	}
}