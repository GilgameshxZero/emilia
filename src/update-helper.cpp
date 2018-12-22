#include "update-helper.h"

namespace Emilia {
	namespace UpdateHelper {
		int pullProc(std::string method, CommandHandlerParam &cmhParam, PullProcParam &pullPP, std::string &message, Rain::SocketManager &sm) {
			std::map<std::string, std::string> &config = *cmhParam.config;

			//parse message based on state of the request

			//relative root for all files to be pushed
			std::string root;
			
			if (method == "pull") {
				root = Rain::pathToAbsolute(config["update-root"]) + config["update-exclusive-dir"] + static_cast<Rain::ClientSocketManager &>(sm).getTargetIP() + "\\";
			} else {
				root = Rain::pathToAbsolute(config["update-root"]);
			}

			//set this to true; if true at the end of function, terminates program for restart
			bool shouldRestart = false;

			if (pullPP.hrPushState == "start") { //receiving filelist
				pullPP.requested.clear();
				pullPP.requestedFiletimes.clear();
				pullPP.unwritable.clear();
				pullPP.noRemove.clear();

				if (method == "push") {
					pullPP.noRemove.insert(cmhParam.notSharedAbsSet.begin(), cmhParam.notSharedAbsSet.end());
				} else if (method == "push-exclusive" || method == "pull") {
					//of the non-specific or specific exclusive files, don't remove those with the same relative path as those in the excIgnVec
					for(int a = 0; a < cmhParam.excIgnVec.size(); a++) {
						pullPP.noRemove.insert(root + cmhParam.excIgnVec[a]);
					}
				}

				//pull commands operate relative the the domain-specific exclusive files, which should all be removed unless matched by hash

				std::stringstream ss;
				ss << message;

				ss >> pullPP.cfiles;
				std::vector<std::pair<FILETIME, std::string>> files;
				for (int a = 0; a < pullPP.cfiles; a++) {
					files.push_back(std::make_pair(FILETIME(), ""));
					ss >> files.back().first.dwHighDateTime >> files.back().first.dwLowDateTime;
					std::getline(ss, files.back().second);
					Rain::strTrimWhite(&files.back().second);
				}
				Rain::tsCout("Received ", method, " request with header with ", std::dec, pullPP.cfiles, " files. Comparing with local files..." + Rain::CRLF);
				std::cout.flush();

				//compare filelist with local hashes (last write time; not crc32) and see which ones need to be updated/deleted
				Rain::tsCout(std::hex, std::setfill('0'));
				for (int a = 0; a < files.size(); a++) {
					FILETIME lastWrite;
					HANDLE hFile;
					//try to get last write time of the file; if it doesn't exist or if the remote time is more recent, request it
					hFile = CreateFile((root + files[a].second).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
					if (GetLastError() == ERROR_FILE_NOT_FOUND) {
						lastWrite.dwHighDateTime = lastWrite.dwLowDateTime = 0;
					} else {
						GetFileTime(hFile, NULL, NULL, &lastWrite);
						CloseHandle(hFile);
					}
					Rain::tsCout(std::setw(8), lastWrite.dwHighDateTime, std::setw(8), lastWrite.dwLowDateTime, " ",
						std::setw(8), files[a].first.dwHighDateTime, std::setw(8), files[a].first.dwLowDateTime, " ");
					if (files[a].first.dwHighDateTime > lastWrite.dwHighDateTime ||
						(files[a].first.dwHighDateTime == lastWrite.dwHighDateTime && files[a].first.dwLowDateTime > lastWrite.dwLowDateTime)) {
						pullPP.requested.push_back(files[a].second);
						pullPP.requestedFiletimes.push_back(files[a].first);
						Rain::tsCout("!");
					} else {
						//hashes match, so add this file to the list of ignored files when we remove files
						pullPP.noRemove.insert(root + files[a].second);
						Rain::tsCout(" ");
					}
					Rain::tsCout(" ", files[a].second, Rain::CRLF);
					std::cout.flush();
				}

				if (pullPP.requested.size() == 0) {
					//if we don't need any files, don't send the request.
					pullPP.hrPushState = "start";
					Rain::tsCout("Local is up-to-date. '", method, "' is unnecessary." + Rain::CRLF);
					std::cout.flush();
					Rain::sendHeadedMessage(sm, method + " 0");
				} else {
					pullPP.cfiles = static_cast<int>(pullPP.requested.size());
					Rain::tsCout(std::dec);
					Rain::tsCout("Requesting ", pullPP.requested.size(), " files in total..." + Rain::CRLF);
					std::cout.flush();

					//send back a list of requested files.
					std::string response = method + " " + Rain::tToStr(pullPP.requested.size()) + "\n";
					for (int a = 0; a < pullPP.requested.size(); a++) {
						response += pullPP.requested[a] + "\n";
					}
					Rain::sendHeadedMessage(sm, &response);

					pullPP.hrPushState = "wait-filelengths";
					pullPP.fileLen.clear();
				}
			} else if (pullPP.hrPushState == "wait-filelengths") {
				std::stringstream ss;
				ss << message;
				pullPP.totalBytes = pullPP.currentBytes = 0;
				for (int a = 0; a < pullPP.cfiles; a++) {
					pullPP.fileLen.push_back(0);
					ss >> pullPP.fileLen.back();
					pullPP.totalBytes += pullPP.fileLen.back();
				}

				//remove all exclusive files except for those matched by hash
				if (method == "push") {
					Rain::rmDirRec(root, &pullPP.noRemove);
				} else if (method == "push-exclusive") {
					Rain::rmDirRec(root, &pullPP.noRemove, &cmhParam.excAbsSet);
				} else if (method == "pull") {
					Rain::rmDirRec(root, &pullPP.noRemove);
				}

				pullPP.hrPushState = "wait-data";
				pullPP.curFile = 0;
				pullPP.curFileLenLeft = -1;

				Rain::tsCout(std::fixed, "Received file lengths. Receiving filedata (", std::setprecision(2), pullPP.totalBytes / 1e6, " MB)..." + Rain::CRLF, std::setfill(' '));
				for (int a = 0; a < pullPP.cfiles; a++) {
					Rain::tsCout(std::setw(8), pullPP.fileLen[a] / 1e6, " MB ", pullPP.requested[a], Rain::CRLF);
				}
				std::cout.flush();

				//temporarily disable logging until file transfer done
				cmhParam.logger->setStdHandleSrc(STD_OUTPUT_HANDLE, false);
			} else if (pullPP.hrPushState == "wait-data") {
				//data is a block of everything in the same order as request, buffered
				if (pullPP.curFileLenLeft == -1) {
					//try to write to a 'new' file on disk at this location; if not possible, save it to a tmp file and note it down
					Rain::createDirRec(Rain::getPathDir(root + pullPP.requested[pullPP.curFile]));
					if (!Rain::isFileWritable(root + pullPP.requested[pullPP.curFile])) {
						pullPP.unwritable.insert(pullPP.curFile);
					}

					pullPP.curFileLenLeft = pullPP.fileLen[pullPP.curFile];
				}

				if (pullPP.unwritable.find(pullPP.curFile) == pullPP.unwritable.end()) {
					Rain::printToFile(root + pullPP.requested[pullPP.curFile], &message, true);
				} else {
					Rain::printToFile(root + pullPP.requested[pullPP.curFile] + config["update-tmp-ext"], &message, true);
				}
				pullPP.currentBytes += message.length();
				if (pullPP.totalBytes > 0) {
					Rain::tsCout("Receiving filedata: ", 100.0 * pullPP.currentBytes / pullPP.totalBytes, "%\r");
					std::cout.flush();
				}
				pullPP.curFileLenLeft -= message.length();

				//done with current file?
				if (pullPP.curFileLenLeft == 0) {
					//modify the last write time of the current file to match that sent in the push header
					HANDLE hFile;
					if (pullPP.unwritable.find(pullPP.curFile) == pullPP.unwritable.end()) {
						hFile = CreateFile((root + pullPP.requested[pullPP.curFile]).c_str(),
							FILE_WRITE_ATTRIBUTES, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
					} else {
						hFile = CreateFile((root + pullPP.requested[pullPP.curFile] + config["update-tmp-ext"]).c_str(),
							FILE_WRITE_ATTRIBUTES, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
					}
					SetFileTime(hFile, NULL, NULL, &pullPP.requestedFiletimes[pullPP.curFile]);
					CloseHandle(hFile);

					//move on to next file
					pullPP.curFile++;
					pullPP.curFileLenLeft = -1;
				}

				if (pullPP.curFile == pullPP.cfiles) {
					//reset state; done with this request
					pullPP.hrPushState = "start";

					cmhParam.logger->setStdHandleSrc(STD_OUTPUT_HANDLE, true);
					Rain::tsCout("\n");

					//if we have unwritable files, note them and start the update script on them; restart the current executable again afterwards
					std::string response;
					response = method + " '" + method + "' completed." + Rain::CRLF;
					Rain::tsCout("'", method, "' completed." + Rain::CRLF);

					//setup update script params
					std::string updateScript = Rain::pathToAbsolute(root + config["update-script"]),
						serverPath = "\"" + Rain::pathToAbsolute(Rain::getExePath()) + "\"";

					//open a script for every unwritable file; these will attempt to start the server multiple times, but only one will succeed
					for (auto it = pullPP.unwritable.begin(); it != pullPP.unwritable.end(); it++) {
						std::string m = "Error: Could not write to " + pullPP.requested[*it] + "." + Rain::CRLF,
							dest = root + pullPP.requested[*it];
						shouldRestart = true;
						ShellExecute(NULL, "open", updateScript.c_str(),
							(serverPath + " \"" + dest + config["update-tmp-ext"] + "\" \"" + dest + "\"").c_str(),
							Rain::getPathDir(updateScript).c_str(), SW_SHOWDEFAULT);
						response += m;
						Rain::tsCout(m);
					}

					if (shouldRestart) {
						std::string m = "Restarting remote to write to locked files..." + Rain::CRLF;
						response += m;
						Rain::tsCout(m);
					}

					std::cout.flush();
					Rain::sendHeadedMessage(sm, &response);
				}
			}

			if (shouldRestart) {
				//restart the application here
				//exit is one of the cleanest ways to do this, short of using many low-level WinAPI calls
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				exit(1);
			}

			return 0;
		}
		int pushProc(std::string method, CommandHandlerParam &cmhParam, PushProcParam &pushPP, std::string &message, Rain::SocketManager &sm) {
			std::map<std::string, std::string> &config = *cmhParam.config;

			//relative root update path to all of the requested files
			std::string root;
			if (method == "push" || method == "pull") {
				root = Rain::pathToAbsolute(config["update-root"]);
			} else if (method == "push-exclusive") {
				root = Rain::pathToAbsolute(config["update-root"]) + config["update-exclusive-dir"] + static_cast<Rain::ClientSocketManager &>(sm).getTargetIP() + "\\";
			}

			if (pushPP.state == "start") {
				if (method == "pull") {
					//send headers, like in push exclusive request
					std::set<std::string> excIgnAbsSet;
					for (int a = 0; a < cmhParam.excIgnVec.size(); a++) {
						excIgnAbsSet.insert(root + cmhParam.excIgnVec[a]);
					}
					std::vector<std::string> exclusive = Rain::getFilesRec(root, "*", &excIgnAbsSet, &cmhParam.excAbsSet);
					Rain::tsCout("Found ", exclusive.size(), " exclusive files to `pull`.", Rain::CRLF);

					//send over list of files and checksums
					Rain::sendHeadedMessage(sm, "pull" + UpdateHelper::generatePushHeader(root, exclusive));
					Rain::tsCout("Sending over 'pull' request with hashes...", Rain::CRLF);
					std::cout.flush();
				}

				//depending on the method, see if we want to process the transition right now
				pushPP.state = "wait-request";
				if (method == "pull") {
					return 0;
				} //otherwise we want to process the current message with the next state
			}

			if (pushPP.state == "wait-request") {
				//HRPush handles a request which lists all the files which the server requests
				std::stringstream ss;
				ss << message;

				ss >> pushPP.cfiles;

				if (pushPP.cfiles == 0) {
					Rain::tsCout("Remote is up-to-date. No '", method, "' necessary." + Rain::CRLF);
					std::cout.flush();

					cmhParam.canAcceptCommand = true;
					cmhParam.canAcceptCommandCV.notify_one();
				} else {
					Rain::tsCout("Received ", pushPP.cfiles, " requested files in response to '", method, "' command. Sending file lengths..." + Rain::CRLF);
					std::cout.flush();

					//temporarily disable logging until file transfer done
					//need to stop here so that logfile doesn't go beyond file length
					cmhParam.logger->setStdHandleSrc(STD_OUTPUT_HANDLE, false);

					std::string tmp;
					std::getline(ss, tmp);
					pushPP.requested.clear();
					for (int a = 0; a < pushPP.cfiles; a++) {
						pushPP.requested.push_back("");
						std::getline(ss, pushPP.requested.back());
						Rain::strTrimWhite(&pushPP.requested.back());
					}

					std::string response = method + " \n";
					std::size_t totalBytes = 0, currentBytes;
					Rain::tsCout(std::fixed, std::setprecision(2), std::setfill(' '));
					for (int a = 0; a < pushPP.requested.size(); a++) {
						currentBytes = Rain::getFileSize(root + pushPP.requested[a]);
						totalBytes += currentBytes;
						response += Rain::tToStr(currentBytes) + "\n";

						Rain::tsCout(std::setw(8), currentBytes / 1e6, " MB ", pushPP.requested[a], Rain::CRLF);
					}
					Rain::sendHeadedMessage(sm, &response);

					Rain::tsCout("Sending filedata (", totalBytes / 1e6, " MB)..." + Rain::CRLF);
					std::cout.flush();

					//move on to send buffered chunks of data from the files, in the same order as the requested files
					int bufferSize = Rain::strToT<int>(config["update-transfer-buffer"]);
					char *buffer = new char[bufferSize];
					std::size_t completedBytes = 0;
					std::string m;
					Rain::tsCout(std::fixed);
					for (int a = 0; a < pushPP.requested.size(); a++) {
						std::ifstream in(root + pushPP.requested[a], std::ios::binary);
						while (in) {
							in.read(buffer, bufferSize);
							m = method + " ";
							m += std::string(buffer, std::size_t(in.gcount()));
							Rain::sendHeadedMessage(sm, &m);
							completedBytes += in.gcount();
							if (totalBytes > 0) {
								Rain::tsCout("Sending filedata: ", 100.0 * completedBytes / totalBytes, "%\r");
							}
							std::cout.flush();
						}
						in.close();
					}
					cmhParam.logger->setStdHandleSrc(STD_OUTPUT_HANDLE, true);
					delete[] buffer;
					Rain::tsCout("\nDone. Waiting for remote response..." + Rain::CRLF);
					std::cout.flush();

					pushPP.state = "wait-complete";
				}
			} else if (pushPP.state == "wait-complete") {
				//everything in response is to be printed to cout and logs
				Rain::tsCout("Remote: ", message);
				std::cout.flush();

				pushPP.state = "start";
				pushPP.requested.clear();

				cmhParam.canAcceptCommand = true;
				cmhParam.canAcceptCommandCV.notify_one();
			}

			return 0;
		}

		std::string generatePushHeader(std::string root, std::vector<std::string> &files) {
			//generate hashes (using last write time instead of crc32)
			std::vector<FILETIME> hash(files.size());
			Rain::tsCout(std::hex, std::setfill('0'));
			for (int a = 0; a < files.size(); a++) {
				HANDLE hFile;
				hFile = CreateFile((root + files[a]).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
				GetFileTime(hFile, NULL, NULL, &hash[a]);
				CloseHandle(hFile);

				Rain::tsCout(std::setw(8), hash[a].dwHighDateTime, std::setw(8), hash[a].dwLowDateTime, " ", files[a], Rain::CRLF);
				std::cout.flush();
			}
			Rain::tsCout(std::dec);

			std::string message = " " + Rain::tToStr(files.size()) + "\n";
			for (int a = 0; a < files.size(); a++) {
				message += Rain::tToStr(hash[a].dwHighDateTime) + " " + Rain::tToStr(hash[a].dwLowDateTime) + " " + files[a] + "\n";
			}

			return message;
		}
	}
}