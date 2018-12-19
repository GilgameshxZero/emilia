#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "command-handler-param.h"

#include <map>
#include <set>

namespace Emilia {
	namespace UpdateServer {
		typedef int(*RequestMethodHandler)(Rain::ServerSocketManager::DelegateHandlerParam &);

		struct ConnectionCallerParam {
			//global config options
			std::map<std::string, std::string> *config;

			//event to break the input loop in main
			HANDLE hInputExitEvent;

			//whether a client is connected; only allow one client to connect at a time
			bool clientConnected;

			Emilia::CommandHandlerParam *cmhParam;

			std::string hrPushState;
		};

		struct ConnectionDelegateParam {
			//parsed first section of the request
			std::string requestMethod;

			//whether current socket is authenticated
			bool authenticated;
		};
	}
}