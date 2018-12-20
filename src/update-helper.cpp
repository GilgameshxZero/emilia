#include "update-helper.h"

namespace Emilia {
	namespace UpdateHelper {
		int ServerPushProc(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam, std::string method) {
			UpdateServer::ConnectionCallerParam &ccParam = *reinterpret_cast<UpdateServer::ConnectionCallerParam *>(ssmdhParam.callerParam);
			UpdateServer::ConnectionDelegateParam &cdParam = *reinterpret_cast<UpdateServer::ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			CommandHandlerParam &cmhParam = *ccParam.cmhParam;
			std::map<std::string, std::string> &config = *cmhParam.config;
			std::string &request = *ssmdhParam.message;

			//parse message based on state of the request

			//relative root for all files to be pushed
			std::string root = Rain::pathToAbsolute(config["update-root"]);

			//set this to true; if true at the end of function, terminates program for restart
			bool shouldRestart = false;

			if (cdParam.hrPushState == "start") { //receiving filelist
				cdParam.requested.clear();
				cdParam.requestedFiletimes.clear();
				cdParam.unwritable.clear();
				cdParam.noRemove.clear();

				if (method == "push") {
					cdParam.noRemove.insert(cmhParam.notSharedAbsSet.begin(), cmhParam.notSharedAbsSet.end());
				}

				std::stringstream ss;
				ss << request;

				ss >> cdParam.cfiles;
				std::vector<std::pair<FILETIME, std::string>> files;
				for (int a = 0; a < cdParam.cfiles; a++) {
					files.push_back(std::make_pair(FILETIME(), ""));
					ss >> files.back().first.dwHighDateTime >> files.back().first.dwLowDateTime;
					std::getline(ss, files.back().second);
					Rain::strTrimWhite(&files.back().second);
				}
				Rain::tsCout("Info: Received ", method, " request with header with ", cdParam.cfiles, " files. Comparing with local files...\r\n");
				fflush(stdout);

				//compare filelist with local hashes (last write time; not crc32) and see which ones need to be updated/deleted
				Rain::tsCout(std::hex, std::setfill('0'));
				for (int a = 0; a < files.size(); a++) {
					FILETIME lastWrite;
					HANDLE hFile;
					hFile = CreateFile((root + files[a].second).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
					GetFileTime(hFile, NULL, NULL, &lastWrite);
					CloseHandle(hFile);
					Rain::tsCout(std::setw(8), lastWrite.dwHighDateTime, std::setw(8), lastWrite.dwLowDateTime, " ",
						std::setw(8), files[a].first.dwHighDateTime, std::setw(8), files[a].first.dwLowDateTime, " ");
					if (files[a].first.dwHighDateTime != lastWrite.dwHighDateTime ||
						files[a].first.dwLowDateTime != lastWrite.dwLowDateTime) {
						cdParam.requested.push_back(files[a].second);
						cdParam.requestedFiletimes.push_back(files[a].first);
						Rain::tsCout("!");
					} else {
						//hashes match, so add this file to the list of ignored files when we remove files
						cdParam.noRemove.insert(root + files[a].second);
						Rain::tsCout(" ");
					}
					Rain::tsCout(" ", files[a].second, "\r\n");
					fflush(stdout);
				}

				if (cdParam.requested.size() == 0) {
					//if we don't need any files, don't send the request.
					cdParam.hrPushState = "start";
					Rain::tsCout("Local is up-to-date. '", method, "' from client is unnecessary.\r\n");
					fflush(stdout);
					Rain::sendHeadedMessage(*ssmdhParam.ssm, method + " 0");
				} else {
					cdParam.cfiles = static_cast<int>(cdParam.requested.size());
					Rain::tsCout(std::dec);
					Rain::tsCout("Info: Requesting ", cdParam.requested.size(), " files in total from client...\r\n");
					fflush(stdout);

					//send back a list of requested files.
					std::string response = method + " " + Rain::tToStr(cdParam.requested.size()) + "\n";
					for (int a = 0; a < cdParam.requested.size(); a++) {
						response += cdParam.requested[a] + "\n";
					}
					Rain::sendHeadedMessage(*ssmdhParam.ssm, &response);

					cdParam.hrPushState = "wait-filelengths";
					cdParam.fileLen.clear();
				}
			} else if (cdParam.hrPushState == "wait-filelengths") {
				std::stringstream ss;
				ss << request;
				cdParam.totalBytes = cdParam.currentBytes = 0;
				for (int a = 0; a < cdParam.cfiles; a++) {
					cdParam.fileLen.push_back(0);
					ss >> cdParam.fileLen.back();
					cdParam.totalBytes += cdParam.fileLen.back();
				}

				//remove all exclusive files except for those matched by hash
				if (method == "push") {
					Rain::rmDirRec(root, &cdParam.noRemove);
				} else if (method == "push-exclusive") {
					Rain::rmDirRec(root, &cdParam.noRemove, &cmhParam.excAbsSet);
				}

				cdParam.hrPushState = "wait-data";
				cdParam.curFile = 0;
				cdParam.curFileLenLeft = -1;

				Rain::tsCout(std::fixed, "Info: Received file lengths from update client. Receiving filedata (", std::setprecision(2), cdParam.totalBytes / 1e6, " MB)...\r\n", std::setfill(' '));
				for (int a = 0; a < cdParam.cfiles; a++) {
					Rain::tsCout(std::setw(8), cdParam.fileLen[a] / 1e6, " MB ", cdParam.requested[a], "\r\n");
				}
				fflush(stdout);
			} else if (cdParam.hrPushState == "wait-data") {
				//data is a block of everything in the same order as request, buffered
				if (cdParam.curFileLenLeft == -1) {
					//try to write to a 'new' file on disk at this location; if not possible, save it to a tmp file and note it down
					Rain::createDirRec(Rain::getPathDir(root + cdParam.requested[cdParam.curFile]));
					if (!Rain::isFileWritable(root + cdParam.requested[cdParam.curFile])) {
						cdParam.unwritable.insert(cdParam.curFile);
					}

					cdParam.curFileLenLeft = cdParam.fileLen[cdParam.curFile];
				}

				if (cdParam.unwritable.find(cdParam.curFile) == cdParam.unwritable.end()) {
					Rain::printToFile(root + cdParam.requested[cdParam.curFile], &request, true);
				} else {
					Rain::printToFile(root + cdParam.requested[cdParam.curFile] + config["update-tmp-ext"], &request, true);
				}
				cdParam.currentBytes += request.length();
				if (cdParam.totalBytes > 0) {
					Rain::tsCout("Receiving filedata: ", 100.0 * cdParam.currentBytes / cdParam.totalBytes, "%\r");
					fflush(stdout);
				}
				cdParam.curFileLenLeft -= request.length();

				//done with current file?
				if (cdParam.curFileLenLeft == 0) {
					//modify the last write time of the current file to match that sent in the push header
					HANDLE hFile;
					if (cdParam.unwritable.find(cdParam.curFile) == cdParam.unwritable.end()) {
						hFile = CreateFile((root + cdParam.requested[cdParam.curFile]).c_str(),
							FILE_WRITE_ATTRIBUTES, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
					} else {
						hFile = CreateFile((root + cdParam.requested[cdParam.curFile] + config["update-tmp-ext"]).c_str(),
							FILE_WRITE_ATTRIBUTES, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
					}
					SetFileTime(hFile, NULL, NULL, &cdParam.requestedFiletimes[cdParam.curFile]);
					CloseHandle(hFile);

					//move on to next file
					cdParam.curFile++;
					cdParam.curFileLenLeft = -1;
				}

				if (cdParam.curFile == cdParam.cfiles) {
					//reset state; done with this request
					cdParam.hrPushState = "start";

					Rain::tsCout("\n");

					//if we have unwritable files, note them and start the update script on them; restart the current executable again afterwards
					std::string response;
					response = method + " '" + method + "' completed.\r\n";
					Rain::tsCout("'", method, "' completed.\r\n");

					//setup update script params
					std::string updateScript = Rain::pathToAbsolute(root + config["update-script"]),
						serverPath = "\"" + Rain::pathToAbsolute(Rain::getExePath()) + "\"";

					//open a script for every unwritable file; these will attempt to start the server multiple times, but only one will succeed
					for (auto it = cdParam.unwritable.begin(); it != cdParam.unwritable.end(); it++) {
						std::string message = "Error: Could not write to " + cdParam.requested[*it] + ".\r\n",
							dest = root + cdParam.requested[*it];
						shouldRestart = true;
						ShellExecute(NULL, "open", updateScript.c_str(), 
							(serverPath + " \"" + dest + config["update-tmp-ext"] + "\" \"" + dest + "\"").c_str(),
							Rain::getPathDir(updateScript).c_str(), SW_SHOWDEFAULT);
						response += message;
						Rain::tsCout(message);
					}

					if (shouldRestart) {
						std::string message = "Restarting server to write to locked files...\r\n";
						response += message;
						Rain::tsCout(message);
					}

					fflush(stdout);
					Rain::sendHeadedMessage(*ssmdhParam.ssm, &response);
				}
			}

			if (shouldRestart) {
				//restart the application here
				//todo: make this cleaner
				exit(0);
				return 1;
			}

			return 0;
		}
		int ClientPushProc(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam, std::string method) {
			UpdateClient::ConnectionHandlerParam &chParam = *reinterpret_cast<UpdateClient::ConnectionHandlerParam *>(csmdhParam.delegateParam);
			std::map<std::string, std::string> &config = *chParam.config;
			std::string &request = *csmdhParam.message;

			//relative root update path to all of the requested files
			std::string root;
			if (method == "push") {
				root = Rain::pathToAbsolute(config["update-root"]);
			} else if (method == "push-exclusive") {
				root = Rain::pathToAbsolute(config["update-root"]) + config["update-exclusive-dir"] + csmdhParam.csm->getTargetIP() + "\\";
			}

			if (chParam.state == "wait-request") {
				//HRPush handles a request which lists all the files which the server requests
				std::stringstream ss;
				ss << request;

				ss >> chParam.cfiles;

				if (chParam.cfiles == 0) {
					Rain::tsCout("Remote is up-to-date. No '", method, "' necessary.\r\n");
					fflush(stdout);
				} else {
					Rain::tsCout("Info: Received ", chParam.cfiles, " requested files in response to '", method, "' command from remote. Sending file lengths...\r\n");
					fflush(stdout);

					std::string tmp;
					std::getline(ss, tmp);
					chParam.requested.clear();
					for (int a = 0; a < chParam.cfiles; a++) {
						chParam.requested.push_back("");
						std::getline(ss, chParam.requested.back());
						Rain::strTrimWhite(&chParam.requested.back());
					}

					std::string response = method + " \n";
					std::size_t totalBytes = 0, currentBytes;
					Rain::tsCout(std::fixed, std::setprecision(2), std::setfill(' '));
					for (int a = 0; a < chParam.requested.size(); a++) {
						currentBytes = Rain::getFileSize(root + chParam.requested[a]);
						totalBytes += currentBytes;
						response += Rain::tToStr(currentBytes) + "\n";

						Rain::tsCout(std::setw(8), currentBytes / 1e6, " MB ", chParam.requested[a], "\r\n");
					}
					Rain::sendHeadedMessage(*csmdhParam.csm, &response);

					Rain::tsCout("Info: Sending filedata (", totalBytes / 1e6, " MB)...\r\n");
					fflush(stdout);

					//move on to send buffered chunks of data from the files, in the same order as the requested files
					int bufferSize = Rain::strToT<int>(config["update-transfer-buffer"]);
					char *buffer = new char[bufferSize];
					std::size_t completedBytes = 0;
					std::string message;
					Rain::tsCout(std::fixed);
					for (int a = 0; a < chParam.requested.size(); a++) {
						std::ifstream in(root + chParam.requested[a], std::ios::binary);
						while (in) {
							in.read(buffer, bufferSize);
							message = method + " ";
							message += std::string(buffer, std::size_t(in.gcount()));
							Rain::sendHeadedMessage(*csmdhParam.csm, &message);
							completedBytes += in.gcount();
							if (totalBytes > 0) {
								Rain::tsCout("Sending filedata: ", 100.0 * completedBytes / totalBytes, "%\r");
							}
							fflush(stdout);
						}
						in.close();
					}
					delete[] buffer;
					Rain::tsCout("\nDone. Waiting for server response...\r\n");
					fflush(stdout);

					chParam.state = "wait-complete";
				}
			} else if (chParam.state == "wait-complete") {
				//everything in response is to be printed to cout and logs
				Rain::tsCout("Remote: ", request);
				fflush(stdout);

				chParam.state = "wait-request";
				chParam.requested.clear();
			}

			return 0;
		}
	}
}