#include "ConnectionHandlers.h"

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		static const std::string headerDelim = "\r\n\r\n";

		int onConnectionInit(void *funcParam) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			chParam.requestLength = 0;
			chParam.doneWaitingEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			return 0;
		}
		int onConnectionExit(void *funcParam) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			return 0;
		}
		int onConnectionProcessMessage(void *funcParam) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			//delegate to request handlers once the message is complete
			//message/request length is at the beginning, as a base-10 string, before a space
			chParam.request += *csmdhParam.message;

			int ret = 0;
			while (true) {
				if (chParam.requestLength == 0) {
					std::size_t firstSpace = chParam.request.find(' ');
					if (firstSpace != std::string::npos) {
						chParam.requestLength = Rain::strToT<std::size_t>(chParam.request.substr(0, firstSpace));
						chParam.request = chParam.request.substr(firstSpace + 1, chParam.request.length());
					}
				}

				//if message is complete
				if (chParam.requestLength != 0 && chParam.request.length() >= chParam.requestLength) {
					std::string fragment = chParam.request.substr(chParam.requestLength, chParam.request.length());
					chParam.request = chParam.request.substr(0, chParam.requestLength);

					int hrReturn = HandleRequest(csmdhParam);
					if (hrReturn < 0 || (hrReturn > 0 && ret >= 0))
						ret = hrReturn;

					chParam.request = fragment;
					chParam.requestLength = 0;
				} else
					break;
			}

			return 0;
		}
	}
}