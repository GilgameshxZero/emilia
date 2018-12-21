#include "update-client.h"

namespace Emilia {
	namespace UpdateClient {
		int onConnect(void *funcParam) {
			Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			chParam.state = "wait-request";

			//authenticate automatically
			Rain::tsCout("Connected with update server. Authenticating..." + Rain::CRLF);
			std::cout.flush();
			chParam.waitingRequests++;
			ResetEvent(chParam.doneWaitingEvent);
			Rain::sendHeadedMessage(*csmdhParam.csm, "authenticate " + chParam.authPass);

			return 0;
		}
		int onDisconnect(void *funcParam) {
			Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			Rain::tsCout("Update server disconnected." + Rain::CRLF);
			std::cout.flush();

			return 0;
		}
		int onMessage(void *funcParam) {
			Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			return HandleRequest(csmdhParam);
		}

		int HandleRequest(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			static const std::map<std::string, RequestMethodHandler> methodHandlerMap{
				{"authenticate", HRAuthenticate}, //validates a socket connection session
				{"push", HRPush},
				{"push-exclusive", HRPushExclusive},
				{"pull", HRPull},
				{"sync", HRSync},
				{"start", HRStart},
				{"stop", HRStop}
			};
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			std::string &request = *csmdhParam.message;

			//takes until the end of string if ' ' can't be found
			size_t firstSpace = request.find(' ');
			chParam.requestMethod = request.substr(0, firstSpace);

			if (firstSpace != request.npos)
				request = request.substr(request.find(' ') + 1, request.length());
			else
				request = "";

			auto handler = methodHandlerMap.find(chParam.requestMethod);
			int handlerRet = 0;
			if (handler != methodHandlerMap.end())
				handlerRet = handler->second(csmdhParam);

			//clear request on exit
			request = "";

			//check if we need to trigger event
			if (chParam.waitingRequests == 0)
				SetEvent(chParam.doneWaitingEvent);

			return handlerRet;
		}

		int HRAuthenticate(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			std::string &request = *csmdhParam.message;
			chParam.waitingRequests--;
			chParam.lastSuccess = 0;
			if (request == "success") {
				Rain::tsCout("Authentication with update server successful." + Rain::CRLF);
			} else if (request == "auth-done") {
				Rain::tsCout("Already authenticated with update server." + Rain::CRLF);
			} else if (request == "fail") {
				Rain::tsCout("Error: Failed to authenticate with update server; disconnecting..." + Rain::CRLF);
				chParam.lastSuccess = -1;
				std::cout.flush();
				return -1;
			} else {
				Rain::tsCout("Error: Unrecognized message from update server; disconnecting..." + Rain::CRLF);
				chParam.lastSuccess = -1;
				std::cout.flush();
				return -1;
			}
			std::cout.flush();
			return 0;
		}
		int HRPush(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			return UpdateHelper::ClientPushProc(csmdhParam, "push");
		}
		int HRPushExclusive(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			return UpdateHelper::ClientPushProc(csmdhParam, "push-exclusive");
		}
		int HRPull(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			return 0;
		}
		int HRSync(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			return 0;
		}
		int HRStart(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			std::string &request = *csmdhParam.message;

			Rain::tsCout("Remote: ", request);
			std::cout.flush();

			return 0;
		}
		int HRStop(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			//nothing reaches here

			return 0;
		}
	}
}