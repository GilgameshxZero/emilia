#pragma once
#include "../rain/src/rain.hpp"

#include "smtp-server-param.hpp"
#include "smtp-external-client.hpp"

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