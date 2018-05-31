#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

#include "InternalConnectionHandlers.h"
#include "ExternalConnectionHandlers.h"

#include <map>
#include <WinDNS.h>

namespace Monochrome3 {
	namespace EmiliaMailServer {
		typedef int(*RequestMethodHandler)(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &);

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

			//used to log socket coms from main
			Rain::LogStream *logger;
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

		int HRREhlo(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRRPreData(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRRData(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);

		int HRRAuthLogin(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRRAuthLoginUsername(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRRAuthLoginPassword(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRRMailFrom(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRRRcptTo(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);

		int HRSAuth(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRSFrom(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRSTo(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRSBody(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
	}
}