#include "update-client.h"

namespace Emilia {
	namespace UpdateClient {
		static const std::string headerDelim = "\r\n\r\n";

		int onConnect(void *funcParam) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			chParam.requestLength = 0;

			//authenticate automatically
			Rain::tsCout("Info: Authenticating with update server...\r\n");
			chParam.waitingRequests++;
			ResetEvent(chParam.doneWaitingEvent);
			Rain::sendBlockMessage(*csmdhParam.csm, "authenticate " + chParam.authPass);

			return 0;
		}
		int onDisconnect(void *funcParam) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			return 0;
		}
		int onMessage(void *funcParam) {
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

			return ret;
		}

		int HandleRequest(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			static const std::map<std::string, RequestMethodHandler> methodHandlerMap{
				{"authenticate", HRAuthenticate}, //validates a socket connection session
				{ "push", HRPush},
				{"push-exclusive", HRPushExclusive},
				{"pull", HRPull},
				{"sync", HRSync},
				{"start", HRStart},
				{"stop", HRStop}
			};
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			//takes until the end of string if ' ' can't be found
			size_t firstSpace = chParam.request.find(' ');
			chParam.requestMethod = chParam.request.substr(0, firstSpace);

			if (firstSpace != chParam.request.npos)
				chParam.request = chParam.request.substr(chParam.request.find(' ') + 1, chParam.request.length());
			else
				chParam.request = "";

			auto handler = methodHandlerMap.find(chParam.requestMethod);
			int handlerRet = 0;
			if (handler != methodHandlerMap.end())
				handlerRet = handler->second(csmdhParam);

			//clear request on exit
			chParam.request = "";

			//check if we need to trigger event
			if (chParam.waitingRequests == 0)
				SetEvent(chParam.doneWaitingEvent);

			return handlerRet;
		}

		int HRAuthenticate(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			chParam.waitingRequests--;
			chParam.lastSuccess = 0;
			if (chParam.request == "success") {
				Rain::tsCout("Info: Authentication with update server successful.\r\n");
			} else if (chParam.request == "auth-done") {
				Rain::tsCout("Info: Already authenticated with update server.\r\n");
			} else if (chParam.request == "fail") {
				Rain::tsCout("Error: Failed to authenticate with update server; disconnecting...\r\n");
				chParam.lastSuccess = -1;
				fflush(stdout);
				return -1;
			} else {
				Rain::tsCout("Error: Unrecognized message from update server; disconnecting...\r\n");
				chParam.lastSuccess = -1;
				fflush(stdout);
				return -1;
			}
			fflush(stdout);
			return 0;
		}
		int HRPush(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			return UpdateHelper::ClientPushProc(csmdhParam, "push");
		}
		int HRPushExclusive(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			return UpdateHelper::ClientPushProc(csmdhParam, "push-exclusive");
		}
		int HRPull(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			return 0;
		}
		int HRSync(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			return 0;
		}
		int HRStart(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			Rain::tsCout("Remote: ", chParam.request);
			fflush(stdout);

			return 0;
		}
		int HRStop(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			//nothing reaches here

			return 0;
		}
	}
}