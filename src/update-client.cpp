#include "update-client.h"

namespace Emilia {
	namespace UpdateClient {
		int onConnect(void *funcParam) {
			Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			//set up some state stuff with pushes and pulls
			chParam.pushPP.state = "start";
			chParam.pullPP.hrPushState = "start";

			//authenticate automatically
			Rain::tsCout("Connected with update server. Authenticating..." + Rain::CRLF);
			std::cout.flush();
			Rain::sendHeadedMessage(*csmdhParam.csm, "authenticate " + chParam.authPass);

			//don't reconnect automatically in general
			csmdhParam.csm->setRetryOnDisconnect(false);

			return 0;
		}
		int onDisconnect(void *funcParam) {
			Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			Rain::tsCout("Update server disconnected." + Rain::CRLF);

			//if it was originally set
			if (csmdhParam.csm->setRetryOnDisconnect(false)) {
				csmdhParam.csm->setRetryOnDisconnect(true);
				Rain::tsCout("Attempting to reconnect after restart..." + Rain::CRLF);
			}

			std::cout.flush();

			//if this flag is set, we want to notify one on disconnect for some command
			if (chParam.notifyCVOnDisconnect) {
				chParam.notifyCVOnDisconnect = false;
				chParam.authCV.notify_one();
			}

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
			else if (chParam.requestMethod == "connect") {
				//a connect message; the only one is that we failed to connect because something else is already connected
				Rain::tsCout("Error: Failed to connect with update server, because another client is already connected." + Rain::CRLF);
				Rain::shutdownSocketSend(csmdhParam.csm->getSocket());
				closesocket(csmdhParam.csm->getSocket());
				chParam.notifyCVOnDisconnect = true;
				return 1;
			}
			else {
				//invalid message, just abort
				Rain::shutdownSocketSend(csmdhParam.csm->getSocket());
				closesocket(csmdhParam.csm->getSocket());
				return 1;
			}

			//clear request on exit
			request = "";

			return handlerRet;
		}

		int HRAuthenticate(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			std::string &request = *csmdhParam.message;
			if (request == "success") {
				Rain::tsCout("Authentication with update server successful." + Rain::CRLF);
			} else if (request == "auth-done") {
				Rain::tsCout("Already authenticated with update server." + Rain::CRLF);
			} else if (request == "fail") {
				Rain::tsCout("Error: Failed to authenticate with update server; disconnecting..." + Rain::CRLF);
				std::cout.flush();
				return -1;
			} else {
				Rain::tsCout("Error: Unrecognized message from update server; disconnecting..." + Rain::CRLF);
				std::cout.flush();
				return -1;
			}
			std::cout.flush();

			chParam.authCV.notify_one();

			return 0;
		}
		int HRPush(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			return UpdateHelper::pushProc("push", *chParam.cmhParam, chParam.pushPP, *csmdhParam.message, *csmdhParam.csm);
		}
		int HRPushExclusive(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			return UpdateHelper::pushProc("push-exclusive", *chParam.cmhParam, chParam.pushPP, *csmdhParam.message, *csmdhParam.csm);
		}
		int HRPull(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			return UpdateHelper::pullProc("pull", *chParam.cmhParam, chParam.pullPP, *csmdhParam.message, *csmdhParam.csm);
		}
		int HRStart(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			std::string &request = *csmdhParam.message;

			Rain::tsCout("Remote response:", Rain::CRLF, request);
			std::cout.flush();

			//mark command complete
			chParam.connectedCommandCV.notify_one();

			return 0;
		}
		int HRStop(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			//we only get "complete messages

			Rain::tsCout("Remote response:", Rain::CRLF, "HTTP & SMTP servers stopped.", Rain::CRLF);
			std::cout.flush();

			//mark command complete
			chParam.connectedCommandCV.notify_one();

			return 0;
		}
	}
}