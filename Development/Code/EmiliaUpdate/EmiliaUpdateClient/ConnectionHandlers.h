#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

#include "RequestHandlers.h"

#include <map>
#include <set>

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		struct ConnectionHandlerParam {

		};

		//handlers for ClientSocketManager
		int onConnectionInit(void *funcParam);
		int onConnectionExit(void *funcParam);
		int onConnectionProcessMessage(void *funcParam);
	}
}