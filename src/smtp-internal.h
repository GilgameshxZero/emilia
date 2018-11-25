/*
Handles requests for internal socket communications, primarily on onDisconnect.
*/

#pragma once

#include "rain-aeternum/rain-libraries.h"

namespace Emilia {
	namespace SMTPServer {
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
	}  // namespace SMTPServer
}  // namespace Emilia