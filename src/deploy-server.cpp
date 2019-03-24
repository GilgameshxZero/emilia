#include "deploy-server.h"

namespace Emilia {
	namespace DeployServer {
		int onConnect(void *funcParam) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			if (ccParam.clientConnected) {
				Rain::tsCout("Update client connection request refused; client already connected." + Rain::CRLF);
				Rain::sendHeadedMessage(*ssmdhParam.ssm, "authenticate refuse");
				Rain::shutdownSocketSend(*ssmdhParam.cSocket);
				closesocket(*ssmdhParam.cSocket);
				return 1; //immediately close recv thread
			}

			ccParam.clientConnected = true;
			Rain::tsCout("Update client connected from " + Rain::getClientNumIP(*ssmdhParam.cSocket) + ". Waiting for authentication (max 5 seconds)..." + Rain::CRLF);

			//create the delegate parameter for the first time
			ConnectionDelegateParam *cdParam = new ConnectionDelegateParam();
			ssmdhParam.delegateParam = reinterpret_cast<void *>(cdParam);

			//initialize cdParam here
			cdParam->authenticated = false;

			//start thread to shutdown connection if not authenticated within 5 seconds
			std::thread([cdParam, ssmdhParam]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(5000));

				if (!cdParam->authenticated) {
					Rain::tsCout("Update client did not successfully authenticate in time. Disconnecting...", Rain::CRLF);
					std::cout.flush();
					closesocket(*ssmdhParam.cSocket);
				}
			}).detach();

			std::cout.flush();
			return 0;
		}
		int onMessage(void *funcParam) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(funcParam);

			return HandleRequest(ssmdhParam);
		}
		int onDisconnect(void *funcParam) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			ccParam.clientConnected = false;
			Rain::tsCout("Update client disconnected." + Rain::CRLF);

			//free the delegate parameter
			delete ssmdhParam.delegateParam;
			std::cout.flush();
			return 0;
		}

		int HandleRequest(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			static const std::map<std::string, RequestMethodHandler> methodHandlerMap{
				{"authenticate", Authenticate}, //validates a socket connection session
				{"sync", Sync},
				{"server", Server},
				{"restart", Restart}
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
					Rain::sendHeadedMessage(*ssmdhParam.ssm, cdParam.requestMethod + " Not authenticated!" + Rain::CRLF);
				} else {
					handlerRet = handler->second(ssmdhParam);
				}
			} else {
				Rain::tsCout("Error: Received unknown method from update client: ", cdParam.requestMethod, "." + Rain::CRLF);
			}

			std::cout.flush();
			return handlerRet;
		}

		int Authenticate(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhp) {
			ConnectionCallerParam &ccp = *reinterpret_cast<ConnectionCallerParam *>(ssmdhp.callerParam);
			ConnectionDelegateParam &cdp = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhp.delegateParam);
			std::string &request = *ssmdhp.message;

			if (cdp.authenticated) {
				Rain::sendHeadedMessage(*ssmdhp.ssm, "authenticate auth-done");
				Rain::tsCout("Update client authenticated already." + Rain::CRLF);
			} else if ((*ccp.config)["deploy-pw"].s() != request) {
				Rain::sendHeadedMessage(*ssmdhp.ssm, "authenticate fail");
				Rain::tsCout("Error: Update client authenticate failed." + Rain::CRLF);

				//wait for client to terminate connection
			} else {
				Rain::sendHeadedMessage(*ssmdhp.ssm, "authenticate success");
				cdp.authenticated = true;
				Rain::tsCout("Update client authenticate successful." + Rain::CRLF);
			}

			std::cout.flush();
			return 0;
		}
		int Sync(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhp) {
			ConnectionCallerParam &ccp = *reinterpret_cast<ConnectionCallerParam *>(ssmdhp.callerParam);
			ConnectionDelegateParam &cdp = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhp.delegateParam);

			//upon first receiving a sync message, first alert the client that we are syncing explicitly with a sync message
			if (cdp.si.stage == "send-emilia") {
				Rain::sendHeadedMessage(*ssmdhp.ssm, "sync");
			}

			//then, the client and the server will share the same routine to manage the sync process
			syncRoutine(ccp.project, *ccp.config, ccp.httpSM, ccp.smtpSM, *ccp.logDeploy, NULL, *ssmdhp.message, *ssmdhp.ssm, cdp.si);

			std::cout.flush();
			return 0;
		}
		int Server(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhp) {
			ConnectionCallerParam &ccp = *reinterpret_cast<ConnectionCallerParam *>(ssmdhp.callerParam);
			ConnectionDelegateParam &cdp = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhp.delegateParam);
			std::string &request = *ssmdhp.message;

			std::string response;
			if (request == "start") {
				Rain::tsCout(std::dec);
				if (!ccp.httpSM->setServerListen(80, 80)) {
					response += "HTTP server listening on port " + Rain::tToStr(ccp.httpSM->getListeningPort()) + "." + Rain::CRLF;
				} else {
					DWORD error = GetLastError();
					response += "Error: could not setup HTTP server listening." + Rain::CRLF;
				}
				if (!ccp.smtpSM->setServerListen(25, 25)) {
					response += "SMTP server listening on port " + Rain::tToStr(ccp.smtpSM->getListeningPort()) + "." + Rain::CRLF;
				} else {
					DWORD error = GetLastError();
					response += "Error: could not setup SMTP server listening." + Rain::CRLF;
				}
			} else if (request == "stop") {
				ccp.httpSM->setServerListen(0, 0);
				ccp.smtpSM->setServerListen(0, 0);
				response += "HTTP & SMTP servers stopped." + Rain::CRLF;
			}
			Rain::sendHeadedMessage(*ssmdhp.ssm, "server " + response);

			std::cout.flush();
			return 0;
		}
		int Restart(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhp) {
			ConnectionCallerParam &ccp = *reinterpret_cast<ConnectionCallerParam *>(ssmdhp.callerParam);
			ConnectionDelegateParam &cdp = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhp.delegateParam);

			prepRestart(ccp.project, ccp.httpSM, ccp.smtpSM);

			//TODO: better exiting
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			UnregisterApplicationRestart();
			exit(1);

			//don't need to send anything
			return 0;
		}

		void syncRoutine(
			std::string project,
			Rain::Configuration &config,
			Rain::ServerManager *httpSM,
			Rain::ServerManager *smtpSM,
			Rain::LogStream &logDeploy,
			Rain::ConditionVariable *completeCV,
			std::string &message,
			Rain::SocketManager &remote,
			SyncInfo &si) {
			if (si.stage == "send-emilia") {
				//send timestamp of process
				time_t timestamp = Rain::getFileLastModifyTime(Rain::getExePath());
				Rain::sendHeadedMessage(remote, "sync " + Rain::tToStr(timestamp));

				//send process binary
				std::string binary;
				Rain::readFileToStr(Rain::getExePath(), binary);
				Rain::sendHeadedMessage(remote, "sync " + binary);

				//update stage
				si.stage = "recv-emilia-timestamp";
			} else if (si.stage == "recv-emilia-timestamp") {
				//receive the timestamp of the remote Emilia
				si.remoteEmiliaTime = Rain::strToT<time_t>(message);

				si.stage = "recv-emilia-binary";
			} else if (si.stage == "recv-emilia-binary") {
				//receive the binary of the remote Emilia
				si.remoteEmiliaBinary = message;

				//send the project index timestamp
				time_t timestamp = Rain::getFileLastModifyTime(project + PROJECT_INDEX);
				Rain::sendHeadedMessage(remote, "sync " + Rain::tToStr(timestamp));

				//send project index
				std::string index;
				Rain::readFileToStr(project + PROJECT_INDEX, index);
				Rain::sendHeadedMessage(remote, "sync " + index);

				si.stage = "recv-index-timestamp";
			} else if (si.stage == "recv-index-timestamp") {
				//receive index timestamp
					si.remoteIndexTime = Rain::strToT<time_t>(message);

				si.stage = "recv-index";
			} else if (si.stage == "recv-index") {
				//receive index
				si.remoteIndex = message;

				//chooose si.index based on the oldest index timestamp
				time_t localIndexTime = Rain::getFileLastModifyTime(project + PROJECT_INDEX);
				if (localIndexTime <= si.remoteIndexTime) {
					si.index = new Rain::Configuration(project + PROJECT_INDEX);
				} else {
					std::string tmpIdxFile = Rain::getTmpFileName();
					std::ofstream out(tmpIdxFile);
					out << si.remoteIndex;
					out.close();
					si.index = new Rain::Configuration(tmpIdxFile);
					DeleteFile(tmpIdxFile.c_str());
				}

				//compute status of each managed file (modified/deleted at time T) w.r.t. index
				std::vector<std::string> files = listProjectFiles(project, config["deploy-ignore"]);

				//true is modified, false is deleted
				for (std::size_t a = 0; a < files.size(); a++) {
					si.status[files[a]] = std::make_pair(true, Rain::getFileLastModifyTime(project + files[a]));
				}
				std::set<std::string> indexSet = si.index->keys();
				for (auto it = indexSet.begin(); it != indexSet.end(); it++) {
					if (si.status.find(*it) == si.status.end()) {
						si.status[*it] = std::make_pair(false, time(0));
					}
				}

				//send the status of all files
				std::string response = "sync " + Rain::tToStr(si.status.size()) + " ";
				for (auto it = si.status.begin(); it != si.status.end(); it++) {
					response += it->first + "\n" + (it->second.first ? "1" : "0") + " " + Rain::tToStr(it->second.second) + "\n";
				}
				Rain::sendHeadedMessage(remote, response);

				si.stage = "unify-status";
			} else if (si.stage == "unify-status") {
				//receive the remote files status
				std::stringstream ss;
				ss << message;

				int remoteStatusSize;
				ss >> remoteStatusSize;

				for (int a = 0; a < remoteStatusSize; a++) {
					std::string filename;
					std::getline(ss, filename);
					Rain::strTrimWhite(&filename);

					int fileStatus;
					time_t fileTime;
					ss >> fileStatus >> fileTime;
					si.remoteStatus[filename] = std::make_pair(fileStatus == 1 ? "true" : "false", fileTime);

					//remove the extra newline
					std::getline(ss, filename);
				}

				//for each file in either status set, apply sync logic to generate a list of files to request and files to delete
				std::set<std::string> unionFiles, toDelete, toRequest;
				for (auto it = si.status.begin(); it != si.status.end(); it++) {
					unionFiles.insert(it->first);
				}
				for (auto it = si.remoteStatus.begin(); it != si.remoteStatus.end(); it++) {
					unionFiles.insert(it->first);
				}
				for (auto it = unionFiles.begin(); it != unionFiles.end(); it++) {
					if (si.status.find(*it) == si.status.end()) {
						toRequest.insert(*it);
					} else if (si.remoteStatus.find(*it) == si.remoteStatus.end()) {
						//do nothing, remote will request
					} else {
						std::pair<bool, time_t> local = si.status[*it],
							remote = si.remoteStatus[*it];

						if (local.first == false && remote.first == false) {
							//do nothing, already deleted
						} else if (local.first == false && remote.first == true) {
							//do nothing, remote will delete
						} else if (local.first == true && remote.first == false) {
							toDelete.insert(*it);
						} else { //both true, so compare times
							if (local.second < remote.second) {
								toRequest.insert(*it);
							} else {
								//do nothing, current version is newer
							}
						}
					}
				}

				//delete the marked files
				for (auto it = toDelete.begin(); it != toDelete.end(); it++) {
					Rain::tsCout("Deleting file ", *it, "...", Rain::CRLF);
					std::cout.flush();
					DeleteFile((project + *it).c_str());
				}

				//send the requested files to remote to request filelength information
				std::string response = "sync " + Rain::tToStr(toRequest.size()) + " ";
				for (auto it = toRequest.begin(); it != toRequest.end(); it++) {
					response += *it + "\n";
				}
				Rain::sendHeadedMessage(remote, response);

				si.stage = "send-filelist";
			} else if (si.stage == "send-filelist") {
				//send back the same list of files but with filelengths
				std::stringstream ss;
				ss << message;
				int files;
				ss >> files;

				std::string response = "sync " + Rain::tToStr(files) + " ";
				for (int a = 0; a < files; a++) {
					std::string file;
					std::getline(ss, file);
					Rain::strTrimWhite(&file);
					si.sending.push_back(std::make_pair(file, Rain::getFileSize(project + file)));
					response += file + "\n" + Rain::tToStr(si.sending.back().second) + "\n";
				}
				Rain::sendHeadedMessage(remote, response);

				si.stage = "recv-header";
			} else if (si.stage == "recv-header") {
				//receive filelengths of requested files
				std::stringstream ss;
				ss << message;
				int files;
				ss >> files;

				for (int a = 0; a < files; a++) {
					std::string file;
					std::getline(ss, file);
					Rain::strTrimWhite(&file);
					std::string len;
					std::getline(ss, len);
					si.receiving.push_back(std::make_pair(file, Rain::strToT<size_t>(len)));
				}

				//start thread to send files in same order as in si.sending
				si.sendThread = std::thread(syncSendFiles, project, config["emilia-buffer"].i(), &si.sending, &remote);

				//prepare to receive files in the same order as in si.receiving
				si.curFile = 0;
				si.curBytes = 0;
				if (si.receiving.size() > 0) {
					Rain::createDirRec(Rain::getPathDir(project + si.receiving[0].first));
					DeleteFile((project + si.receiving[0].first).c_str());
					Rain::tsCout("Receiving file 1 of ", si.receiving.size(), " (", si.receiving[0].second, " bytes @ ", si.receiving[0].first, ")...", Rain::CRLF);
					std::cout.flush();
				}
				si.stage = "recv-files";
			} else if (si.stage == "recv-files") {
				//receive buffered files, assuming none are unwritable
				if (si.receiving.size() > 0) {
					Rain::printToFile(project + si.receiving[si.curFile].first, &message, true);
					si.curBytes += message.length();
				}

				//done with current file? (>= to be safe)
				if (si.receiving.size() > 0 && si.curBytes >= si.receiving[si.curFile].second) {
					//modify the last write time of the current file to match that sent in the push header
					struct _utimbuf ut;
					ut.actime = ut.modtime = si.remoteStatus[si.receiving[si.curFile].first].second;
					_utime((project + si.receiving[si.curFile].first).c_str(), &ut);

					//move on to next file
					si.curFile++;
					si.curBytes = 0;
					if (si.curFile != si.receiving.size()) {
						//ensure directory containing file exists
						Rain::createDirRec(Rain::getPathDir(project + si.receiving[si.curFile].first));
						DeleteFile((project + si.receiving[si.curFile].first).c_str());
						Rain::tsCout("Receiving file ", si.curFile + 1, " of ", si.receiving.size(), " (", si.receiving[si.curFile].second, " bytes @ ", si.receiving[si.curFile].first, ")...", Rain::CRLF);
						std::cout.flush();
					}
				}

				//done with receiving?
				if (si.curFile == si.receiving.size()) {
					Rain::tsCout("Finished receiving files!", Rain::CRLF);

					//wait until sending done
					si.sendThread.join();

					//update index
					std::vector<std::string> projectFiles = listProjectFiles(project, config["deploy-ignore"]);
					std::ofstream indexOut(project + PROJECT_INDEX, std::ios::binary);
					for (int a = 0; a < projectFiles.size(); a++) {
						indexOut << projectFiles[a] << Rain::LF;
					}
					indexOut.close();
					Rain::tsCout("Index created and sync completed.", Rain::CRLF);
					std::cout.flush();

					//now, check if Emilia needs to be overwritten
					if (si.remoteEmiliaTime > Rain::getFileLastModifyTime(Rain::getExePath())) {
						Rain::tsCout("Restarting to update Emilia...", Rain::CRLF);
						std::cout.flush();

						std::string emiliaTmpFile = Rain::getExePath() + ".tmp";
						std::ofstream out(emiliaTmpFile, std::ios::binary);
						out << si.remoteEmiliaBinary;
						out.close();
						struct _utimbuf ut;
						ut.actime = ut.modtime = si.remoteEmiliaTime;
						_utime(emiliaTmpFile.c_str(), &ut);

						prepRestart(project, httpSM, smtpSM, emiliaTmpFile);

						//TODO: better exiting
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
						UnregisterApplicationRestart();
						exit(1);
					}

					//reset si & cleanup
					si = SyncInfo();
					if (completeCV != NULL) {
						completeCV->notify_one();
					}
				}
			}
		}

		std::vector<std::string> listProjectFiles(std::string project, Rain::Configuration &ignored) {
			std::set<std::string> ignoredSet = ignored.keys(), realIgnore;
			ignoredSet.insert(PROJECT_INDEX);
			for (auto it = ignoredSet.begin(); it != ignoredSet.end(); it++) {
				realIgnore.insert(Rain::pathToAbsolute(project + *it));
			}
			return Rain::getFilesRec(project, "*", &realIgnore);
		}

		void syncSendFiles(std::string project, int bufferSize, std::vector<std::pair<std::string, size_t>> *sending, Rain::SocketManager *remote) {
			std::vector<std::pair<std::string, size_t>> &sref = *sending;
			char *buffer = new char[bufferSize];
			Rain::tsCout(std::fixed);
			for (size_t a = 0; a < sref.size(); a++) {
				Rain::tsCout("Sending file ", a + 1, " of ", sref.size(), " (", sref[a].second, " bytes @ ", sref[a].first, ")...", Rain::CRLF);
				std::cout.flush();
				std::ifstream in(project + sref[a].first, std::ios::binary);
				size_t readFromFile = 0;
				while (in && readFromFile < sref[a].second) {
					in.read(buffer, bufferSize);
					//don't send more than necessary if file has become larger
					Rain::sendHeadedMessage(*remote, "sync " + std::string(buffer, min(sref[a].second - readFromFile, size_t(in.gcount()))));
					readFromFile += size_t(in.gcount());
				}
				in.close();

				//TODO: clean this up
				//if file has become smaller since we last checked, send junk bytes
				if (readFromFile < sref[a].second) {
					Rain::sendHeadedMessage(*remote, "sync " + std::string(sref[a].second - readFromFile, '\n'));
				}

				//if file is empty, send a junk message to prompt the receiving end to start next file
				if (sref[a].second == 0) {
					Rain::sendHeadedMessage(*remote, "sync");
				}
			}
			//if no files, send empty to prompt action from other side
			if (sref.size() == 0) {
				Rain::sendHeadedMessage(*remote, "sync");
			}
			remote->blockForMessageQueue(0);
			Rain::tsCout("Finished sending files!", Rain::CRLF);
		}
	}
}