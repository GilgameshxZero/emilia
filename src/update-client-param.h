#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "command-handler-param.h"
#include "update-helper-param.h"

#include <map>
#include <set>

namespace Emilia {
	//forward declaration for circular reference
	struct CommandHandlerParam;

	namespace UpdateClient {
		typedef int(*RequestMethodHandler)(Rain::ClientSocketManager::DelegateHandlerParam &);

		struct ConnectionHandlerParam {
			Emilia::CommandHandlerParam *cmhParam;

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

			UpdateHelper::PushProcParam pushPP;
			UpdateHelper::PullProcParam pullPP;
		};
	}
}