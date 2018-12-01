#include "update-server.h"

namespace Emilia {
	namespace UpdateServer {
		static const std::string headerDelim = "\r\n\r\n";

		int onConnect(void *funcParam) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			if (ccParam.clientConnected) {
				Rain::tsCout("Info: Update client connection request refused; client already connected.\r\n");
				fflush(stdout);
				return 1; //immediately terminate
			}

			ccParam.clientConnected = true;
			ccParam.hrPushState = "start";
			Rain::tsCout("Info: Update client connected.\r\n");
			fflush(stdout);

			//create the delegate parameter for the first time
			ConnectionDelegateParam *cdParam = new ConnectionDelegateParam();
			ssmdhParam.delegateParam = reinterpret_cast<void *>(cdParam);

			//initialize cdParam here
			cdParam->authenticated = false;

			return 0;
		}
		int onMessage(void *funcParam) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			//delegate to request handlers once the message is complete
			//message/request length is at the beginning, as a base-10 string, before a space
			cdParam.request += *ssmdhParam.message;

			int ret = 0;
			while (true) {
				if (cdParam.requestLength == 0) {
					std::size_t firstSpace = cdParam.request.find(' ');
					if (firstSpace != std::string::npos) {
						cdParam.requestLength = Rain::strToT<std::size_t>(cdParam.request.substr(0, firstSpace));
						cdParam.request = cdParam.request.substr(firstSpace + 1, cdParam.request.length());
					}
				}

				//if message is complete
				if (cdParam.requestLength != 0 && cdParam.request.length() >= cdParam.requestLength) {
					std::string fragment = cdParam.request.substr(cdParam.requestLength, cdParam.request.length());
					cdParam.request = cdParam.request.substr(0, cdParam.requestLength);

					int hrReturn = HandleRequest(ssmdhParam);
					if (hrReturn < 0 || (hrReturn > 0 && ret >= 0))
						ret = hrReturn;
					if (ret < 0)
						return ret;

					cdParam.request = fragment;
					cdParam.requestLength = 0;
				} else
					break;
			}

			return ret;
		}
		int onDisconnect(void *funcParam) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			ccParam.clientConnected = false;
			Rain::tsCout("Info: Update client disconnected.\r\n");
			fflush(stdout);

			//free the delegate parameter
			delete ssmdhParam.delegateParam;
			return 0;
		}
		
		int HandleRequest(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
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

			//takes until the end of string if ' ' can't be found
			size_t firstSpace = cdParam.request.find(' ');
			cdParam.requestMethod = cdParam.request.substr(0, firstSpace);

			if (firstSpace != cdParam.request.npos)
				cdParam.request = cdParam.request.substr(cdParam.request.find(' ') + 1, cdParam.request.length());
			else
				cdParam.request = "";

			auto handler = methodHandlerMap.find(cdParam.requestMethod);
			int handlerRet = 0;
			if (handler != methodHandlerMap.end()) {
				//block if not authenticated
				if (cdParam.requestMethod != "authenticate" && !cdParam.authenticated) {
					Rain::sendBlockMessage(*ssmdhParam.ssm, cdParam.requestMethod + " not authenticated\r\n");
				} else {
					handlerRet = handler->second(ssmdhParam);
				}
			} else {
				Rain::tsCout("Error: Received unknown method from update client: ", cdParam.requestMethod, ".\r\n");
			}

			//clear request on exit
			cdParam.request = "";

			return handlerRet;
		}
		
		int HRAuthenticate(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			if (cdParam.authenticated) {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "authenticate auth-done");
				Rain::tsCout("Info: Update client authenticated already.\r\n");
			} else if (ccParam.config->at("emilia-auth-pass") != cdParam.request) {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "authenticate fail");
				Rain::tsCout("Error: Update client authenticate fail.\r\n");
				fflush(stdout);
				return -1;
			} else {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "authenticate success");
				cdParam.authenticated = true;
				Rain::tsCout("Info: Update client authenticate success.\r\n");
			}

			fflush(stdout);
			return 0;
		}
		int HRPush(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			return Emilia::UpdateHelper::ServerPushProc(ssmdhParam, "push");
		}
		int HRPushExclusive(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			return Emilia::UpdateHelper::ServerPushProc(ssmdhParam, "push-exclusive");
		}
		int HRPull(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			return 0;
		}
		int HRSync(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			return 0;
		}
		int HRStart(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			CommandHandlerParam &cmhParam = *reinterpret_cast<CommandHandlerParam *>(ccParam.cmhParam);

			//send the command line back as it is over the socket, which should get printed by the client
			std::string response;

			if (!cmhParam.httpSM->setServerListen(80, 80)) {
				response = "Info: HTTP server listening on port " + Rain::tToStr(cmhParam.httpSM->getListeningPort()) + "." + "\r\n";
				Rain::tsCout("Info: HTTP server listening on port ", cmhParam.httpSM->getListeningPort(), ".", "\r\n");
			} else {
				DWORD error = GetLastError();
				response = "Error: could not setup HTTP server listening.\r\n";
				Rain::errorAndCout(error, "Error: could not setup HTTP server listening.");
			}
			Rain::sendBlockMessage(*ssmdhParam.ssm, "start " + response);

			if (!cmhParam.smtpSM->setServerListen(25, 25)) {
				response = "Info: SMTP server listening on port " + Rain::tToStr(cmhParam.smtpSM->getListeningPort()) + "." + "\r\n";
				Rain::tsCout("Info: SMTP server listening on port ", cmhParam.smtpSM->getListeningPort(), ".", "\r\n");
			} else {
				DWORD error = GetLastError();
				response = "Error: could not setup SMTP server listening.\r\n";
				Rain::errorAndCout(error, "Error: could not setup SMTP server listening.");
			}
			fflush(stdout);
			Rain::sendBlockMessage(*ssmdhParam.ssm, "start " + response);

			return 0;
		}
		int HRStop(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			CommandHandlerParam &cmhParam = *reinterpret_cast<CommandHandlerParam *>(ccParam.cmhParam);

			cmhParam.httpSM->setServerListen(0, 0);
			cmhParam.smtpSM->setServerListen(0, 0);

			return 0;
		}
	}
}