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
				handlerRet = handler->second(ssmdhParam);
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
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			CommandHandlerParam &cmhParam = *ccParam.cmhParam;
			std::map<std::string, std::string> &config = *cmhParam.config;

			//parse message based on state of the request
			static int cfiles;
			static std::vector<std::string> requested;
			static std::vector<std::size_t> fileLen;
			static int curFile;
			static std::size_t curFileLenLeft;
			static std::set<int> unwritable;
			static std::set<std::string> noRemove;

			static std::size_t totalBytes = 0, currentBytes = 0;

			std::string root = Rain::pathToAbsolute(config["update-root"]);

			bool shouldRestart = false;

			if (ccParam.hrPushState == "start") { //receiving filelist
				requested.clear();
				unwritable.clear();
				noRemove.clear();
				noRemove.insert(cmhParam.notSharedAbsSet.begin(), cmhParam.notSharedAbsSet.end());

				std::stringstream ss;
				ss << cdParam.request;

				ss >> cfiles;
				std::vector<std::pair<unsigned int, std::string>> files;
				for (int a = 0; a < cfiles; a++) {
					files.push_back(std::make_pair(0, ""));
					ss >> files.back().first;
					std::getline(ss, files.back().second);
					Rain::strTrimWhite(&files.back().second);
				}
				Rain::tsCout("Info: Received push request with header with ", cfiles, " files. Comparing with local files...\r\n");
				fflush(stdout);

				//compare filelist with local checksums and see which ones need to be updated/deleted
				Rain::tsCout(std::hex);
				for (int a = 0; a < files.size(); a++) {
					unsigned int crc32 = Rain::checksumFileCRC32(root + files[a].second);
					Rain::tsCout(std::setw(8), crc32, " ", std::setw(8), files[a].first, " ", files[a].second, " ");
					if (files[a].first != crc32) {
						requested.push_back(files[a].second);
						Rain::tsCout("DIFF");
					} else {
						//CRC32s match, so add this file to the list of ignored files when we remove files
						noRemove.insert(root + files[a].second);
					}
					Rain::tsCout("\r\n");
				}
				fflush(stdout);

				if (requested.size() == 0) {
					//if we don't need any files, don't send the request.
					ccParam.hrPushState = "start";
					Rain::tsCout("Local is up-to-date. 'push' from client is unnecessary.");
					fflush(stdout);
					Rain::sendBlockMessage(*ssmdhParam.ssm, "push 0");
				} else {
					cfiles = static_cast<int>(requested.size());
					Rain::tsCout(std::dec);
					Rain::tsCout("Info: Requesting ", requested.size(), " files in total from client.\r\n");
					fflush(stdout);

					//send back a list of requested files.
					std::string response = "push " + Rain::tToStr(requested.size()) + "\n";
					for (int a = 0; a < requested.size(); a++) {
						response += requested[a] + "\n";
					}
					Rain::sendBlockMessage(*ssmdhParam.ssm, &response);

					ccParam.hrPushState = "wait-filelengths";
					fileLen.clear();
				}
			} else if (ccParam.hrPushState == "wait-filelengths") {
				Rain::tsCout("Info: Received filelengths from update client. Receiving file lengths...\r\n");
				fflush(stdout);

				std::stringstream ss;
				ss << cdParam.request;
				totalBytes = currentBytes = 0;
				for (int a = 0; a < cfiles; a++) {
					fileLen.push_back(0);
					ss >> fileLen.back();
					totalBytes += fileLen.back();
				}
				Rain::tsCout("Info: Receiving filedata (", totalBytes / 1e6, " MB)...\r\n");

				//remove all shared files except for those matched by CRC32
				Rain::rmDirRec(root, &noRemove);

				ccParam.hrPushState = "wait-data";
				curFile = 0;
				curFileLenLeft = -1;

				Rain::tsCout(std::fixed);
			} else if (ccParam.hrPushState == "wait-data") {
				//data is a block of everything in the same order as request, buffered
				if (curFileLenLeft == -1) {
					//try to write to a 'new' file on disk at this location; if not possible, save it to a tmp file and note it down
					Rain::createDirRec(Rain::getPathDir(root + requested[curFile]));
					if (!Rain::isFileWritable(root + requested[curFile])) {
						unwritable.insert(curFile);
					}

					curFileLenLeft = fileLen[curFile];
				}

				if (unwritable.find(curFile) == unwritable.end()) {
					Rain::printToFile(root + requested[curFile], &cdParam.request, true);
				} else {
					Rain::printToFile(root + requested[curFile] + config["update-tmp-ext"], &cdParam.request, true);
				}
				currentBytes += cdParam.request.length();
				Rain::tsCout("Receiving filedata: ", std::setw(6), 100.0 * currentBytes / totalBytes, "%\r");
				fflush(stdout);
				curFileLenLeft -= cdParam.request.length();

				if (curFileLenLeft == 0) {
					curFile++;
					curFileLenLeft = -1;
				}

				if (curFile == cfiles) {
					//reset state; done with this request
					ccParam.hrPushState = "start";

					Rain::tsCout("\n");

					//if we have unwritable files, note them; if that includes this executable, restart this executable
					std::string response;
					response = "push ";
					for (auto it = unwritable.begin(); it != unwritable.end(); it++) {
						std::string message = "Error: Could not write to " + requested[*it] + ".";
						if (Rain::pathToAbsolute(root + requested[*it]) == Rain::pathToAbsolute(Rain::getExePath())) {
							//the current file is part of the unwritable files
							message += " This is the current executable. Restarting to write to executable; will restart automatically after.";
							shouldRestart = true;
						}
						message += "\r\n";
						response += message;
						Rain::tsCout(message);
					}
					fflush(stdout);

					Rain::sendBlockMessage(*ssmdhParam.ssm, &response);
				}
			}

			if (shouldRestart) {
				//need to restart current executable after replacing it with the tmp file
				std::string updateScript = Rain::pathToAbsolute(root + config["update-script"]),
					cmdLine = "\"" + Rain::pathToAbsolute(Rain::getExePath()) + "\" \"" + Rain::pathToAbsolute(Rain::getExePath() + config["update-tmp-ext"]) + "\"";
				ShellExecute(NULL, "open", updateScript.c_str(), cmdLine.c_str(), Rain::getPathDir(updateScript).c_str(), SW_SHOWDEFAULT);
				exit(0);
				return 1;
			}

			return 0;
		}
		int HRPushExclusive(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			return 0;
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