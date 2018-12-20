#include "update-server.h"

namespace Emilia {
	namespace UpdateServer {
		static const std::string headerDelim = "\r\n\r\n";

		int onConnect(void *funcParam) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			if (ccParam.clientConnected) {
				Rain::tsCout("Info: Update client connection request refused; client already connected.\r\n");
				fflush(stdout);
				return 1; //immediately terminate
			}

			ccParam.clientConnected = true;
			Rain::tsCout("Info: Update client connected.\r\n");
			fflush(stdout);

			//create the delegate parameter for the first time
			ConnectionDelegateParam *cdParam = new ConnectionDelegateParam();
			ssmdhParam.delegateParam = reinterpret_cast<void *>(cdParam);

			//if we do any push requests, start it out as "start" state
			cdParam->hrPushState = "start";

			//initialize cdParam here
			cdParam->authenticated = false;

			return 0;
		}
		int onMessage(void *funcParam) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			return HandleRequest(ssmdhParam);
		}
		int onDisconnect(void *funcParam) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			ccParam.clientConnected = false;
			Rain::tsCout("Info: Update client disconnected.\r\n");
			fflush(stdout);

			//free the delegate parameter
			delete ssmdhParam.delegateParam;
			return 0;
		}
		
		int HandleRequest(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			static const std::map<std::string, RequestMethodHandler> methodHandlerMap{
				{"authenticate", HRAuthenticate}, //validates a socket connection session
				{"push", HRPush},
				{"push-exclusive", HRPushExclusive},
				{"pull", HRPull},
				{"sync", HRSync},
				{"start", HRStart},
				{"stop", HRStop}
			};
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			std::string &request = *ssmdhParam.message;

			//takes until the end of string if ' ' can't be found
			size_t firstSpace = request.find(' ');
			cdParam.requestMethod = request.substr(0, firstSpace);

			if (firstSpace != request.npos)
				request = request.substr(request.find(' ') + 1, request.length());
			else
				request = "";

			auto handler = methodHandlerMap.find(cdParam.requestMethod);
			int handlerRet = 0;
			if (handler != methodHandlerMap.end()) {
				//block if not authenticated
				if (cdParam.requestMethod != "authenticate" && !cdParam.authenticated) {
					Rain::sendHeadedMessage(*ssmdhParam.ssm, cdParam.requestMethod + " not authenticated\r\n");
				} else {
					handlerRet = handler->second(ssmdhParam);
				}
			} else {
				Rain::tsCout("Error: Received unknown method from update client: ", cdParam.requestMethod, ".\r\n");
			}

			return handlerRet;
		}
		
		int HRAuthenticate(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			std::string &request = *ssmdhParam.message;

			if (cdParam.authenticated) {
				Rain::sendHeadedMessage(*ssmdhParam.ssm, "authenticate auth-done");
				Rain::tsCout("Info: Update client authenticated already.\r\n");
			} else if (ccParam.config->at("emilia-auth-pass") != request) {
				Rain::sendHeadedMessage(*ssmdhParam.ssm, "authenticate fail");
				Rain::tsCout("Error: Update client authenticate fail.\r\n");
				fflush(stdout);
				return -1;
			} else {
				Rain::sendHeadedMessage(*ssmdhParam.ssm, "authenticate success");
				cdParam.authenticated = true;
				Rain::tsCout("Info: Update client authenticate success.\r\n");
			}

			fflush(stdout);
			return 0;
		}
		int HRPush(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			return Emilia::UpdateHelper::ServerPushProc(ssmdhParam, "push");
		}
		int HRPushExclusive(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			return Emilia::UpdateHelper::ServerPushProc(ssmdhParam, "push-exclusive");
		}
		int HRPull(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			return 0;
		}
		int HRSync(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			return 0;
		}
		int HRStart(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			CommandHandlerParam &cmhParam = *reinterpret_cast<CommandHandlerParam *>(ccParam.cmhParam);

			//send the command line back as it is over the socket, which should get printed by the client
			std::string response;
			Rain::tsCout(std::dec);
			if (!cmhParam.httpSM->setServerListen(80, 80)) {
				response = "Info: HTTP server listening on port " + Rain::tToStr(cmhParam.httpSM->getListeningPort()) + "." + "\r\n";
				Rain::tsCout("Info: HTTP server listening on port ", cmhParam.httpSM->getListeningPort(), ".", "\r\n");
			} else {
				DWORD error = GetLastError();
				response = "Error: could not setup HTTP server listening.\r\n";
				Rain::errorAndCout(error, "Error: could not setup HTTP server listening.");
			}
			Rain::sendHeadedMessage(*ssmdhParam.ssm, "start " + response);

			if (!cmhParam.smtpSM->setServerListen(25, 25)) {
				response = "Info: SMTP server listening on port " + Rain::tToStr(cmhParam.smtpSM->getListeningPort()) + "." + "\r\n";
				Rain::tsCout("Info: SMTP server listening on port ", cmhParam.smtpSM->getListeningPort(), ".", "\r\n");
			} else {
				DWORD error = GetLastError();
				response = "Error: could not setup SMTP server listening.\r\n";
				Rain::errorAndCout(error, "Error: could not setup SMTP server listening.");
			}
			fflush(stdout);
			Rain::sendHeadedMessage(*ssmdhParam.ssm, "start " + response);

			return 0;
		}
		int HRStop(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			CommandHandlerParam &cmhParam = *reinterpret_cast<CommandHandlerParam *>(ccParam.cmhParam);

			cmhParam.httpSM->setServerListen(0, 0);
			cmhParam.smtpSM->setServerListen(0, 0);

			return 0;
		}
	}
}