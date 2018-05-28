#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

#include "ConnectionHandlers.h"

#include <shellapi.h>
#include <string>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		typedef int(*RequestMethodHandler)(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
	}
}