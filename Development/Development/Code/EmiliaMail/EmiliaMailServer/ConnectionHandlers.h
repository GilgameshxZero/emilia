#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

#include "RequestHandlers.h"

#include <map>

namespace Monochrome3 {
	namespace EmiliaMailServer {
		struct ConnectionCallerParam {
			std::map<std::string, std::string> *config;
		};

		struct ConnectionDelegateParam {
			//type of connection; either "send" or "receive", depending on whether we should send/recv an email based on this request
			std::string cType;

			//accumulated request
			std::string request;

			//if send request, length of whole request block
			std::size_t requestLength;

			//if recv request, the current request handler
			RequestMethodHandler recvHandler;
		};

		int onConnect(void *param);
		int onMessage(void *param);
		int onDisconnect(void *param);
	}
}