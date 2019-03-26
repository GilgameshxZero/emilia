#pragma once
#include "rain-aeternum/rain-libraries.h"

#include "smtp-server-param.h"
#include "smtp-external-client.h"

#include <WinDNS.h>
#pragma comment(lib, "Dnsapi.lib")

namespace Emilia {
	namespace SMTPServer {
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