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

			//password for authentication on reconnect
			std::string authPass;

			//condition variable triggered once on each authentication response
			Rain::ConditionVariable authCV;

			//triggered when 'sync'-like command is complete
			Rain::ConditionVariable connectedCommandCV;

			UpdateHelper::PushProcParam pushPP;
			UpdateHelper::PullProcParam pullPP;
		};
	}
}