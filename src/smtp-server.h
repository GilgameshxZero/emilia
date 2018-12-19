#pragma once
#pragma comment(lib, "Dnsapi.lib")

#include "rain-aeternum/rain-libraries.h"

#include "smtp-external-client.h"

#include <map>
#include <WinDNS.h>

namespace Emilia {
	namespace SMTPServer {
		typedef int(*RequestMethodHandler)(Rain::ServerSocketManager::DelegateHandlerParam &);

		struct RecvConnectionDelegateParam {
			//the current request handler
			RequestMethodHandler reqHandler;

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
			std::map<std::string, std::string> *config;

			//username/password pairs in base-64
			std::map<std::string, std::string> b64Users;

			//used to log socket coms from main
			Rain::LogStream *logger;

			//total number of connected clients
			int connectedClients;
		};

		struct ConnectionDelegateParam {
			//accumulated request
			std::string request;

			RecvConnectionDelegateParam rcd;
		};

		int onConnect(void *param);
		int onMessage(void *param);
		int onDisconnect(void *param);

		//Ehlo manages both EHLO and HELO requests
		int HRREhlo(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);

		int HRRPreData(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
		int HRRData(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);

		int HRRAuthLogin(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
		int HRRAuthLoginUsername(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
		int HRRAuthLoginPassword(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
		int HRRMailFrom(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
		int HRRRcptTo(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);
	}  // namespace SMTPServer
}  // namespace Emilia