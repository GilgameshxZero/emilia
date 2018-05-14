#pragma once

#include "../../Common/RainLibrary3/RainLibraries.h"

#include "BuildID.h"

#include <map>
#include <set>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		struct ConnectionCallerParam {
			//global config options
			std::map<std::string, std::string> *config;
		};

		struct ConnectionDelegateParam {
			
		};

		//handlers for RecvThread
		void onConnectionInit(void *funcParam);
		void onConnectionExit(void *funcParam);
		int onConnectionProcessMessage(void *funcParam);
	}
}