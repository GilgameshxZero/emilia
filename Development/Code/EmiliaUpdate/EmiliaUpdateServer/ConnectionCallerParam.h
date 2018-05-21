#pragma once

#include "ServerStatus.h"

#include <map>
#include <string>
#include <vector>
#include <Windows.h>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		struct ConnectionCallerParam {
			//global config options
			std::map<std::string, std::string> *config;

			//event to break the input loop in main
			HANDLE hInputExitEvent;

			//status of child servers
			std::vector<ServerStatus> serverStatus;

			//whether a client is connected; only allow one client to connect at a time
			bool clientConnected;
		};
	}
}