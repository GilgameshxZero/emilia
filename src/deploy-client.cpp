#include "deploy-client.h"

namespace Emilia {
	namespace DeployClient {
		int onConnect(void *funcParam) {
			Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			//authenticate automatically
			Rain::tsCout("Connected with update server. Authenticating..." + Rain::CRLF);
			Rain::sendHeadedMessage(*csmdhParam.csm, "authenticate " + chParam.authPass);

			//don't reconnect automatically in general
			csmdhParam.csm->setRetryOnDisconnect(false);

			std::cout.flush();
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

			//if this flag is set, we want to notify one on disconnect for some command
			if (chParam.notifyCVOnDisconnect) {
				chParam.notifyCVOnDisconnect = false;
				chParam.authCV.notify_one();
			}

			std::cout.flush();
			return 0;
		}
		int onMessage(void *funcParam) {
			Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			static const std::map<std::string, RequestMethodHandler> methodHandlerMap{
				{"authenticate", authenticate}, //validates a socket connection session
				{"sync", sync},
				{"server", server}
			};
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
			if (handler != methodHandlerMap.end()) {
				handlerRet = handler->second(csmdhParam);
			} else {
				//invalid message, just abort
				Rain::shutdownSocketSend(csmdhParam.csm->getSocket());
				closesocket(csmdhParam.csm->getSocket());
				return 1;
			}

			//clear request on exit
			request = "";

			return handlerRet;
		}

		int authenticate(Rain::ClientSocketManager::DelegateHandlerParam &csmdhp) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhp.delegateParam);
			std::string &request = *csmdhp.message;

			if (request == "success") {
				Rain::tsCout("Authentication with deploy server successful.", Rain::CRLF);
			} else if (request == "auth-done") {
				Rain::tsCout("Already authenticated with deploy server.", Rain::CRLF);
			} else if (request == "fail") {
				Rain::tsCout("Error: Failed to authenticate with update server; disconnecting...", Rain::CRLF);
				chParam.authCV.notify_one();
				return -1;
			} else if (request == "refuse") {
				Rain::tsCout("Error: Server refused connection because another client was connected already; disconnecting...", Rain::CRLF);
				chParam.authCV.notify_one();
				return -1;
			} else {
				Rain::tsCout("Error: Unrecognized message from update server; disconnecting...", Rain::CRLF);
				chParam.authCV.notify_one();
				return -1;
			}
			chParam.authCV.notify_one();

			std::cout.flush();
			return 0;
		}
		int sync(Rain::ClientSocketManager::DelegateHandlerParam &csmdhp) {
			ConnectionHandlerParam &chp = *reinterpret_cast<ConnectionHandlerParam *>(csmdhp.delegateParam);

			//the client and server share the same sync routine
			DeployServer::syncRoutine(chp.project, *chp.config, chp.httpSM, chp.smtpSM, *chp.logDeploy, &chp.connectedCommandCV, *csmdhp.message, *csmdhp.csm, chp.si);

			std::cout.flush();
			return 0;
		}
		int server(Rain::ClientSocketManager::DelegateHandlerParam &csmdhp) {
			ConnectionHandlerParam &chp = *reinterpret_cast<ConnectionHandlerParam *>(csmdhp.delegateParam);
			std::string &request = *csmdhp.message;

			Rain::tsCout("Remote response:", Rain::CRLF, request);
			chp.connectedCommandCV.notify_one();

			std::cout.flush();
			return 0;
		}
	}
}