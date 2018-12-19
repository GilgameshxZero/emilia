/*
Manages CSM to external SMTP servers; used by the internal client.
*/

#pragma once

#include "rain-aeternum/rain-libraries.h"

namespace Emilia {
	namespace SMTPServer {
		typedef int(*ExternalRequestMethodHandler)(Rain::ClientSocketManager::DelegateHandlerParam &);

		struct ExternalConnectionParam {
			//event is set when communications are finished
			HANDLE hFinish;

			//result of communications, as a code
			//0 is success
			int status;

			//request handler
			ExternalRequestMethodHandler reqHandler;

			//accumulated messages
			std::string request;

			//email data
			std::string *to, *from, *data;
		};

		int onExternalConnect(void *param);
		int onExternalMessage(void *param);
		int onExternalDisconnect(void *param);

		int EHREhlo(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int EHRMailFrom(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int EHRRcptTo(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int EHRPreData(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int EHRData(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
		int EHRQuit(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam);
	}  // namespace SMTPServer
}  // namespace Emilia