#pragma once

#include "rain-aeternum/rain-libraries.h"

#include <map>
#include <set>

namespace Emilia {
	namespace UpdateClient {
		typedef int(*RequestMethodHandler)(Rain::ClientSocketManager::DelegateHandlerParam &);

		struct ConnectionHandlerParam {
			//global config
			std::map<std::string, std::string> *config;

			//parsed first section of the request
			std::string requestMethod;

			//a variable, manually set by the CSM thread, which will trigger an event when it reaches 0
			//decreases by 1 every time a response is complete, or by a trigger specific to each method
			int waitingRequests;

			//event triggered by waitingRequests
			HANDLE doneWaitingEvent;

			//the success of the last request, as a code
			//nonzero for error
			int lastSuccess;

			//password for authentication on reconnect
			std::string authPass;
		};
	}
}