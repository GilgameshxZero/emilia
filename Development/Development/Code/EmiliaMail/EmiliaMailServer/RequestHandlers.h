#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

namespace Monochrome3 {
	namespace EmiliaMailServer {
		typedef int(*RequestMethodHandler)(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &);

		int HRRecvEhlo(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRRecvData(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRRecvSendMail(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRRecvQuit(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRRecvAuthLogin(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);

		int HRSendRequest(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
	}
}