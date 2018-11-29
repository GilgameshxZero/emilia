#include "update-client.h"

namespace Emilia {
	namespace UpdateClient {
		static const std::string headerDelim = "\r\n\r\n";

		int onConnect(void *funcParam) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			chParam.requestLength = 0;
			chParam.doneWaitingEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

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
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			std::map<std::string, std::string> &config = *chParam.config;

			static std::string state = "wait-request";

			static int cfiles;
			static std::vector<std::string> requested;
			std::string root = Rain::pathToAbsolute(config["update-root"]);

			if (state == "wait-request") {
				//HRPush handles a request which lists all the files which the server requests
				std::stringstream ss;
				ss << chParam.request;

				ss >> cfiles;

				if (cfiles == 0) {
					Rain::tsCout("Remote is up-to-date. No 'push' necessary.\r\n");
					fflush(stdout);
				} else {
					std::string tmp;
					std::getline(ss, tmp);
					requested.clear();
					for (int a = 0; a < cfiles; a++) {
						requested.push_back("");
						std::getline(ss, requested.back());
						Rain::strTrimWhite(&requested.back());
					}

					std::string response = "push \n";
					std::size_t totalBytes = 0, currentBytes;
					for (int a = 0; a < requested.size(); a++) {
						currentBytes = Rain::getFileSize(root + requested[a]);
						totalBytes += currentBytes;
						response += Rain::tToStr(currentBytes) + "\n";
					}
					Rain::sendBlockMessage(*csmdhParam.csm, &response);

					Rain::tsCout("Info: Received ", cfiles, " requested files in response to 'push' command from remote. Sending filedata (", totalBytes / 1e6, " MB)...\r\n");
					fflush(stdout);

					//move on to send buffered chunks of data from the files, in the same order as the requested files
					int bufferSize = Rain::strToT<int>(config["update-transfer-buffer"]);
					char *buffer = new char[bufferSize];
					std::size_t completedBytes = 0;
					std::string message;
					Rain::tsCout(std::fixed);
					for (int a = 0; a < requested.size(); a++) {
						std::ifstream in(root + requested[a], std::ios::binary);
						while (in) {
							in.read(buffer, bufferSize);
							message = "push ";
							message += std::string(buffer, std::size_t(in.gcount()));
							Rain::sendBlockMessage(*csmdhParam.csm, &message);
							completedBytes += in.gcount();
							Rain::tsCout("Sending filedata: ", std::setw(6), 100.0 * completedBytes / totalBytes, "%\r");
							fflush(stdout);
						}
						in.close();
					}
					delete[] buffer;
					Rain::tsCout("\nDone. Waiting for server response...\r\n");
					fflush(stdout);

					state = "wait-complete";
				}
			} else if (state == "wait-complete") {
				//everything in response is to be printed to cout and logs
				Rain::tsCout(chParam.request);
				fflush(stdout);

				state = "wait-request";
				requested.clear();
			}

			return 0;
		}
		int HRPushExclusive(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			std::map<std::string, std::string> &config = *chParam.config;

			static std::string state = "wait-request";

			static int cfiles;
			static std::vector<std::string> requested;
			std::string root = Rain::pathToAbsolute(config["update-root"]) + config["update-exclusive-dir"] + csmdhParam.csm->getTargetIP() + "\\";

			if (state == "wait-request") {
				//HRPush handles a request which lists all the files which the server requests
				std::stringstream ss;
				ss << chParam.request;

				ss >> cfiles;

				if (cfiles == 0) {
					Rain::tsCout("Remote is up-to-date. No 'push-exclusive' necessary.\r\n");
					fflush(stdout);
				} else {
					std::string tmp;
					std::getline(ss, tmp);
					requested.clear();
					for (int a = 0; a < cfiles; a++) {
						requested.push_back("");
						std::getline(ss, requested.back());
						Rain::strTrimWhite(&requested.back());
					}

					std::string response = "push-exclusive \n";
					std::size_t totalBytes = 0, currentBytes;
					for (int a = 0; a < requested.size(); a++) {
						currentBytes = Rain::getFileSize(root + requested[a]);
						totalBytes += currentBytes;
						response += Rain::tToStr(currentBytes) + "\n";
					}
					Rain::sendBlockMessage(*csmdhParam.csm, &response);

					Rain::tsCout("Info: Received ", cfiles, " requested files in response to 'push-exclusive' command from remote. Sending filedata (", totalBytes / 1e6, " MB)...\r\n");
					fflush(stdout);

					//move on to send buffered chunks of data from the files, in the same order as the requested files
					int bufferSize = Rain::strToT<int>(config["update-transfer-buffer"]);
					char *buffer = new char[bufferSize];
					std::size_t completedBytes = 0;
					std::string message;
					Rain::tsCout(std::fixed);
					for (int a = 0; a < requested.size(); a++) {
						std::ifstream in(root + requested[a], std::ios::binary);
						while (in) {
							in.read(buffer, bufferSize);
							message = "push-exclusive ";
							message += std::string(buffer, std::size_t(in.gcount()));
							Rain::sendBlockMessage(*csmdhParam.csm, &message);
							completedBytes += in.gcount();
							Rain::tsCout("Sending filedata: ", std::setw(6), 100.0 * completedBytes / totalBytes, "%\r");
							fflush(stdout);
						}
						in.close();
					}
					delete[] buffer;
					Rain::tsCout("\nDone. Waiting for server response...\r\n");
					fflush(stdout);

					state = "wait-complete";
				}
			} else if (state == "wait-complete") {
				//everything in response is to be printed to cout and logs
				Rain::tsCout(chParam.request);
				fflush(stdout);

				state = "wait-request";
				requested.clear();
			}

			return 0;
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