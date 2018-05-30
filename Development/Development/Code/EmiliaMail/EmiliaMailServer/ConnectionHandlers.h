#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

#include "RequestHandlers.h"

#include <map>

namespace Monochrome3 {
	namespace EmiliaMailServer {
		struct SendConnectionDelegateParam {
			//if send request, length of whole request block
			std::size_t requestLength;

			//the current request handler
			RequestMethodHandler reqHandler;

			//fields about the email
			std::string to;
			std::string from;
			std::string data;
		};

		struct RecvConnectionDelegateParam {
			//the current request handler
			RequestMethodHandler reqHandler;

			//the current logged in user
			std::string b64User;

			//the MAIL FROM field
			std::string mailFrom;

			//if recv request, the users to whom to deliver mail to
			std::set <std::string> rcptTo;

			//email data
			std::string mailData;
		};

		struct ConnectionCallerParam {
			std::map<std::string, std::string> *config;

			//username/password pairs in base-64
			std::map<std::string, std::string> b64Users;
		};

		struct ConnectionDelegateParam {
			//type of connection; either "send" or "receive", depending on whether we should send/recv an email based on this request
			std::string cType;

			//accumulated request
			std::string request;

			//parameters related to each type of request to come into this server
			SendConnectionDelegateParam scd;
			RecvConnectionDelegateParam rcd;
		};

		int onConnect(void *param);
		int onMessage(void *param);
		int onDisconnect(void *param);
	}
}