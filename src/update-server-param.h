#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "command-handler-param.h"

#include <map>
#include <set>

namespace Emilia {
	namespace UpdateServer {
		typedef int(*RequestMethodHandler)(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &);

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
			//accumulated request from messages
			std::string request;

			//length of the request
			std::size_t requestLength;

			//parsed first section of the request
			std::string requestMethod;

			//whether current socket is authenticated
			bool authenticated;
		};
	}
}