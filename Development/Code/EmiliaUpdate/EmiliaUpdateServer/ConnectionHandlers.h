#pragma once
#include "../../Common/RainLibrary3/RainLibraries.h"

#include "ConnectionCallerParam.h"
#include "ConnectionDelegateParam.h"
#include "RequestHandlers.h"

#include <map>
#include <set>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		//handlers for RecvThread
		void onConnectionInit(void *funcParam);
		void onConnectionExit(void *funcParam);
		int onConnectionProcessMessage(void *funcParam);
	}
}