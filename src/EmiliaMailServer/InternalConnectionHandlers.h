/*
Handles requests for internal socket communications, primarily on onDisconnect.
*/

#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

namespace Monochrome3 {
	namespace EmiliaMailServer {
		struct InternalConnectionParam {
			//event which is set when the internal communication finishes
			HANDLE hFinish;

			//accumulated messages
			std::string request;

			//set by the caller, info about the internal request
			std::string toAddress;
		};

		int onInternalConnect(void *param);
		int onInternalMessage(void *param);
		int onInternalDisconnect(void *param);
	}
}