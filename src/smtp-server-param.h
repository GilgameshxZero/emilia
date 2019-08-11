#pragma once
#include "rain-aeternum/rain-libraries.h"

namespace Emilia {
	namespace SMTPServer {
		typedef int(*RequestMethodHandler)(Rain::ServerSocketManager::DelegateHandlerParam &);

		struct RecvConnectionDelegateParam {
			//the current request handler
			RequestMethodHandler reqHandler = NULL;

			//the current logged in user
			std::string b64User;

			//the MAIL FROM field
			std::string mailFrom;

			//if recv request, the users to whom to deliver mail to
			std::set<std::string> rcptTo;

			//email data
			std::string mailData;
		};

		struct ConnectionCallerParam {
			std::string project;
			Rain::Configuration *config;
			Rain::LogStream *logSMTP;

			//total number of connected clients
			int connectedClients = 0;

			//username/password pairs in base-64
			std::map<std::string, std::string> b64Users;
		};

		struct ConnectionDelegateParam {
			//accumulated request
			std::string request;

			RecvConnectionDelegateParam rcd;
		};
	}
}