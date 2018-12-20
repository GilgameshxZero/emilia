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

			//used to access config
			Emilia::CommandHandlerParam *cmhParam;
		};

		struct ConnectionDelegateParam {
			//parsed first section of the request
			std::string requestMethod;

			//whether current socket is authenticated
			bool authenticated;

			//state of the push/push-exclusive request
			std::string hrPushState;

			//info about files of the push/push-exclusive request
			std::size_t totalBytes, currentBytes, curFileLenLeft;

			int cfiles, curFile;
			std::vector<std::string> requested;
			std::vector<FILETIME> requestedFiletimes;
			std::vector<std::size_t> fileLen;
			std::set<int> unwritable;
			std::set<std::string> noRemove;
		};
	}
}