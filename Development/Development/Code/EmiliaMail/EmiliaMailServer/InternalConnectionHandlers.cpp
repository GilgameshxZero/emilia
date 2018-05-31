#include "InternalConnectionHandlers.h"

namespace Monochrome3 {
	namespace EmiliaMailServer {
		int onInternalConnect(void *param) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
			InternalConnectionParam &icParam = *reinterpret_cast<InternalConnectionParam *>(csmdhParam.delegateParam);
			return 0;
		}
		int onInternalMessage(void *param) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
			InternalConnectionParam &icParam = *reinterpret_cast<InternalConnectionParam *>(csmdhParam.delegateParam);

			icParam.request += *csmdhParam.message;

			return 0;
		}
		int onInternalDisconnect(void *param) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
			InternalConnectionParam &icParam = *reinterpret_cast<InternalConnectionParam *>(csmdhParam.delegateParam);

			//output status of the request
			int status = Rain::strToT<int>(icParam.request);
			if (status == 0)
				Rain::tsCout("Success: Sent email to ", icParam.toAddress, ".\r\n");
			else
				Rain::tsCout("Failure: Didn't send email to ", icParam.toAddress, ".\r\n");

			//set event for waiting threads
			SetEvent(icParam.hFinish);

			return 0;
		}
	}
}