#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

#include "RequestHandlers.h"

#include <map>
#include <set>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		struct ServerStatus {
			//file location of server//absolute or full path
			std::string path;

			//status of server
			//running or stopped
			std::string status;

			//handle to process
			HANDLE process;

			//handles to stdio
			HANDLE stdInRd, stdInWr, stdOutRd, stdOutWr;
		};

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

		//handlers for RecvThread
		int onConnect(void *funcParam);
		int onMessage(void *funcParam);
		int onDisconnect(void *funcParam);
	}
}