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

				if (method == "push") {
					noRemove.insert(cmhParam.notSharedAbsSet.begin(), cmhParam.notSharedAbsSet.end());
				}

				std::stringstream ss;
				ss << request;

				ss >> cfiles;
				std::vector<std::pair<unsigned int, std::string>> files;
				for (int a = 0; a < cfiles; a++) {
					files.push_back(std::make_pair(0, ""));
					ss >> files.back().first;
					std::getline(ss, files.back().second);
					Rain::strTrimWhite(&files.back().second);
				}
				Rain::tsCout("Info: Received ", method, " request with header with ", cfiles, " files. Comparing with local files...\r\n");
				fflush(stdout);

				//compare filelist with local checksums and see which ones need to be updated/deleted
				Rain::tsCout(std::hex);
				for (int a = 0; a < files.size(); a++) {
					unsigned int crc32 = Rain::checksumFileCRC32(root + files[a].second);
					Rain::tsCout(std::setw(8), crc32, " ", std::setw(8), files[a].first, " ");
					if (files[a].first != crc32) {
						requested.push_back(files[a].second);
						Rain::tsCout("! ");
					} else {
						//CRC32s match, so add this file to the list of ignored files when we remove files
						noRemove.insert(root + files[a].second);
						Rain::tsCout("  ");
					}
					Rain::tsCout(files[a].second, "\r\n");
					fflush(stdout);
				}

				if (requested.size() == 0) {
					//if we don't need any files, don't send the request.
					ccParam.hrPushState = "start";
					Rain::tsCout("Local is up-to-date. '", method, "' from client is unnecessary.\r\n");
					fflush(stdout);
					Rain::sendHeadedMessage(*ssmdhParam.ssm, method + " 0");
				} else {
					cfiles = static_cast<int>(requested.size());
					Rain::tsCout(std::dec);
					Rain::tsCout("Info: Requesting ", requested.size(), " files in total from client...\r\n");
					fflush(stdout);

					//send back a list of requested files.
					std::string response = method + " " + Rain::tToStr(requested.size()) + "\n";
					for (int a = 0; a < requested.size(); a++) {
						response += requested[a] + "\n";
					}
					Rain::sendHeadedMessage(*ssmdhParam.ssm, &response);

					ccParam.hrPushState = "wait-filelengths";
					fileLen.clear();
				}
			} else if (ccParam.hrPushState == "wait-filelengths") {
				std::stringstream ss;
				ss << request;
				totalBytes = currentBytes = 0;
				for (int a = 0; a < cfiles; a++) {
					fileLen.push_back(0);
					ss >> fileLen.back();
					totalBytes += fileLen.back();
				}

				//remove all exclusive files except for those matched by CRC32
				if (method == "push") {
					Rain::rmDirRec(root, &noRemove);
				} else if (method == "push-exclusive") {
					Rain::rmDirRec(root, &noRemove, &cmhParam.excAbsSet);
				}

				ccParam.hrPushState = "wait-data";
				curFile = 0;
				curFileLenLeft = -1;

				Rain::tsCout(std::fixed, "Info: Received file lengths from update client. Receiving filedata (", std::setprecision(2), totalBytes / 1e6, " MB)...\r\n");
				for (int a = 0; a < cfiles; a++) {
					Rain::tsCout(std::setw(8), fileLen[a] / 1e6, " MB ", requested[a], "\r\n");
				}
				fflush(stdout);
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
					Rain::printToFile(root + requested[curFile], &request, true);
				} else {
					Rain::printToFile(root + requested[curFile] + config["update-tmp-ext"], &request, true);
				}
				currentBytes += request.length();
				Rain::tsCout("Receiving filedata: ", 100.0 * currentBytes / totalBytes, "%\r");
				fflush(stdout);
				curFileLenLeft -= request.length();

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
					response = method + " '" + method + "' completed.\r\n";
					Rain::tsCout("'", method, "' completed.\r\n");
					for (auto it = unwritable.begin(); it != unwritable.end(); it++) {
						std::string message = "Error: Could not write to " + requested[*it] + ".";
						if (Rain::pathToAbsolute(root + requested[*it]) == Rain::pathToAbsolute(Rain::getExePath())) {
							//the current file is part of the unwritable files
							message += " Restarting to write to executable...";
							shouldRestart = true;
						}
						message += "\r\n";
						response += message;
						Rain::tsCout(message);
					}
					fflush(stdout);

					Rain::sendHeadedMessage(*ssmdhParam.ssm, &response);
				}
			}

			if (shouldRestart) {
				//need to restart current executable after replacing it with the tmp file
				std::string updateScript = Rain::pathToAbsolute(root + config["update-script"]),
					cmdLine = "\"" + Rain::pathToAbsolute(Rain::getExePath()) + "\" \"" + Rain::pathToAbsolute(Rain::getExePath() + config["update-tmp-ext"]) + "\"";
				ShellExecute(NULL, "open", updateScript.c_str(), cmdLine.c_str(), Rain::getPathDir(updateScript).c_str(), SW_SHOWDEFAULT);

				//todo: make this cleaner
				Sleep(1000);
				exit(0);
				return 1;
			}

			return 0;
		}
		int ClientPushProc(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam, std::string method) {
			UpdateClient::ConnectionHandlerParam &chParam = *reinterpret_cast<UpdateClient::ConnectionHandlerParam *>(csmdhParam.delegateParam);
			std::map<std::string, std::string> &config = *chParam.config;
			std::string &request = *csmdhParam.message;

			static std::string state = "wait-request";

			static int cfiles;
			static std::vector<std::string> requested;
			std::string root;
			if (method == "push") {
				root = Rain::pathToAbsolute(config["update-root"]);
			} else if (method == "push-exclusive") {
				root = Rain::pathToAbsolute(config["update-root"]) + config["update-exclusive-dir"] + csmdhParam.csm->getTargetIP() + "\\";
			}

			if (state == "wait-request") {
				//HRPush handles a request which lists all the files which the server requests
				std::stringstream ss;
				ss << request;

				ss >> cfiles;

				if (cfiles == 0) {
					Rain::tsCout("Remote is up-to-date. No '", method, "' necessary.\r\n");
					fflush(stdout);
				} else {
					Rain::tsCout("Info: Received ", cfiles, " requested files in response to '", method, "' command from remote. Sending file lengths...\r\n");
					fflush(stdout);

					std::string tmp;
					std::getline(ss, tmp);
					requested.clear();
					for (int a = 0; a < cfiles; a++) {
						requested.push_back("");
						std::getline(ss, requested.back());
						Rain::strTrimWhite(&requested.back());
					}

					std::string response = method + " \n";
					std::size_t totalBytes = 0, currentBytes;
					Rain::tsCout(std::fixed, std::setprecision(2));
					for (int a = 0; a < requested.size(); a++) {
						currentBytes = Rain::getFileSize(root + requested[a]);
						totalBytes += currentBytes;
						response += Rain::tToStr(currentBytes) + "\n";

						Rain::tsCout(std::setw(8), currentBytes / 1e6, " MB ", requested[a], "\r\n");
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
					for (int a = 0; a < requested.size(); a++) {
						std::ifstream in(root + requested[a], std::ios::binary);
						while (in) {
							in.read(buffer, bufferSize);
							message = method + " ";
							message += std::string(buffer, std::size_t(in.gcount()));
							Rain::sendHeadedMessage(*csmdhParam.csm, &message);
							completedBytes += in.gcount();
							Rain::tsCout("Sending filedata: ", 100.0 * completedBytes / totalBytes, "%\r");
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
				Rain::tsCout("Remote: ", request);
				fflush(stdout);

				state = "wait-request";
				requested.clear();
			}

			return 0;
		}
	}
}