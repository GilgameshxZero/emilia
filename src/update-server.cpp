#include "update-server.h"

namespace Emilia {
	namespace UpdateServer {
		int onConnect(void *funcParam) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			if (ccParam.clientConnected) {
				Rain::tsCout("Update client connection request refused; client already connected." + Rain::CRLF);
				std::cout.flush();
				return 1; //immediately terminate
			}

			ccParam.clientConnected = true;
			Rain::tsCout("Update client connected." + Rain::CRLF);
			std::cout.flush();

			//create the delegate parameter for the first time
			ConnectionDelegateParam *cdParam = new ConnectionDelegateParam();
			ssmdhParam.delegateParam = reinterpret_cast<void *>(cdParam);

			//if we do any push requests, start it out as "start" state
			cdParam->pullPP.hrPushState = "start";
			cdParam->pushPP.state = "start";

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
			Rain::tsCout("Update client disconnected." + Rain::CRLF);
			std::cout.flush();

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
					Rain::sendHeadedMessage(*ssmdhParam.ssm, cdParam.requestMethod + " not authenticated" + Rain::CRLF);
				} else {
					handlerRet = handler->second(ssmdhParam);
				}
			} else {
				Rain::tsCout("Error: Received unknown method from update client: ", cdParam.requestMethod, "." + Rain::CRLF);
			}

			return handlerRet;
		}
		
		int HRAuthenticate(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			std::string &request = *ssmdhParam.message;

			if (cdParam.authenticated) {
				Rain::sendHeadedMessage(*ssmdhParam.ssm, "authenticate auth-done");
				Rain::tsCout("Update client authenticated already." + Rain::CRLF);
			} else if (ccParam.config->at("emilia-auth-pass") != request) {
				Rain::sendHeadedMessage(*ssmdhParam.ssm, "authenticate fail");
				Rain::tsCout("Error: Update client authenticate fail." + Rain::CRLF);
				std::cout.flush();
				return -1;
			} else {
				Rain::sendHeadedMessage(*ssmdhParam.ssm, "authenticate success");
				cdParam.authenticated = true;
				Rain::tsCout("Update client authenticate success." + Rain::CRLF);
			}

			std::cout.flush();
			return 0;
		}
		int HRPush(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			return Emilia::UpdateHelper::pullProc("push", *ccParam.cmhParam, cdParam.pullPP, *ssmdhParam.message, *ssmdhParam.ssm);
		}
		int HRPushExclusive(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			return Emilia::UpdateHelper::pullProc("push-exclusive", *ccParam.cmhParam, cdParam.pullPP, *ssmdhParam.message, *ssmdhParam.ssm);
		}
		int HRPull(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			return Emilia::UpdateHelper::pushProc("pull", *ccParam.cmhParam, cdParam.pushPP, *ssmdhParam.message, *ssmdhParam.ssm);
		}
		int HRStart(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			CommandHandlerParam &cmhParam = *reinterpret_cast<CommandHandlerParam *>(ccParam.cmhParam);

			//send the command line back as it is over the socket, which should get printed by the client
			std::string response;
			Rain::tsCout(std::dec);
			if (!cmhParam.httpSM->setServerListen(80, 80)) {
				response = "HTTP server listening on port " + Rain::tToStr(cmhParam.httpSM->getListeningPort()) + "." + Rain::CRLF;
				Rain::tsCout("HTTP server listening on port ", cmhParam.httpSM->getListeningPort(), ".", Rain::CRLF);
			} else {
				DWORD error = GetLastError();
				response = "Error: could not setup HTTP server listening." + Rain::CRLF;
				Rain::errorAndCout(error, "Error: could not setup HTTP server listening.");
			}
			Rain::sendHeadedMessage(*ssmdhParam.ssm, "start " + response);

			if (!cmhParam.smtpSM->setServerListen(25, 25)) {
				response = "SMTP server listening on port " + Rain::tToStr(cmhParam.smtpSM->getListeningPort()) + "." + Rain::CRLF;
				Rain::tsCout("SMTP server listening on port ", cmhParam.smtpSM->getListeningPort(), ".", Rain::CRLF);
			} else {
				DWORD error = GetLastError();
				response = "Error: could not setup SMTP server listening." + Rain::CRLF;
				Rain::errorAndCout(error, "Error: could not setup SMTP server listening.");
			}
			std::cout.flush();
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