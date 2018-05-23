#include "RequestHandlers.h"

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		int HandleRequest(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam) {
			static const std::map<std::string, RequestMethodHandler> methodHandlerMap{
				{"authenticate", HRAuthenticate}, //validates a socket connection session
				{"prod-upload", HRProdUpload}, //updates server-side production files with client files
				{"prod-download", HRProdDownload}, //request for current production files to client
				{"prod-stop", HRProdStop}, //stop all server services
				{"prod-start", HRProdStart}, //start all server services
				{"sync-stop", HRSyncStop}, //stops constant updates and enables other commands
				{"sync-start", HRSyncStart}, //request constant updates for changed production files; disables other commands except sync-stop
			};
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ltrfdParam.delegateParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);

			//takes until the end of string if ' ' can't be found
			size_t firstSpace = cdParam.request.find(' ');
			cdParam.requestMethod = cdParam.request.substr(0, firstSpace);

			if (firstSpace != cdParam.request.npos)
				cdParam.request = cdParam.request.substr(cdParam.request.find(' ') + 1, cdParam.request.length());
			else
				cdParam.request = "";

			auto handler = methodHandlerMap.find(cdParam.requestMethod);
			int handlerRet = 0;
			if (handler != methodHandlerMap.end())
				handlerRet = handler->second(ltrfdParam);

			//clear request on exit
			cdParam.request = "";

			return handlerRet;
		}

		int HRAuthenticate(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam) {
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ltrfdParam.delegateParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);
			if (cdParam.authenticated) {
				Rain::sendBlockText(*ltrfdParam.cSocket, "Already authenticated.");
			} else if (ccParam.config->at("client-auth-pass") != cdParam.request) {
				Rain::sendBlockText(*ltrfdParam.cSocket, "Authentication failed.");
			} else {
				Rain::sendBlockText(*ltrfdParam.cSocket, "Authentication success.");
				cdParam.authenticated = true;
			}

			return 0;
		}
		int HRProdUpload(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam) {
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ltrfdParam.delegateParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);
			if (!cdParam.authenticated) {
				Rain::sendBlockText(*ltrfdParam.cSocket, "Not yet authenticated.");
				return 0;
			}
			//we assume the server is shut down at this point

			//clear these variables on request end
			//unprocessed request buffer, since request might be blocked
			static std::string reqAcc = "",
				//path to the current exe, which might cause problems when updating
				thisPath = Rain::getFullPathStr(Rain::getExecutablePath());
			//position of end of the header block
			static std::size_t headerLen = 0,
				//how many bytes of current file we have on disk already
				curFileDone = 0;
			//ordered list of files
			static std::vector<std::pair<std::string, std::size_t>> header;
			//file currently processing
			static int curFile = 0;
			//flag for if we need to use CRH
			static bool delayExeWrite = false;

			if (headerLen == 0) {
				std::size_t emptyLine = Rain::rabinKarpMatch(cdParam.request, "\r\n");
				if (emptyLine == -1) {
					reqAcc += cdParam.request;
				} else {
					headerLen = reqAcc.length() + emptyLine + 1; //+1 because \r\n is two characters

					//process header
					std::stringstream headerSS;
					headerSS << reqAcc << cdParam.request.substr(0, emptyLine);
					reqAcc = cdParam.request.substr(emptyLine + 2, cdParam.request.length());

					int n;
					headerSS >> n;
					header.resize(n);
					for (int a = 0; a < n; a++) {
						std::getline(headerSS, header[a].first, '\n'); //should extract up until and including \n
						Rain::strTrim(header[a].first); //clean up \r
						static std::string tmp;
						std::getline(headerSS, tmp, '\n'); //should extract up until and including \n
						header[a].second = Rain::strToT<std::size_t>(tmp);
					}

					//nuke prod (which should work, except for the current executable)
					Rain::recursiveRmDir((*ccParam.config)["prod-root-dir"]);
				}
			}

			//if the header is processed, append requests to correct files
			if (headerLen != 0) {
				reqAcc += cdParam.request;
				std::size_t curFileReqBlock = min(reqAcc.length(), header[curFile].second - curFileDone);

				//as long as the file is modifiable (i.e. it's not the current exe), update it
				//otherwise, save the data in a temp file, and mark a flag to delay the copying of the temp file to the exe file with CRHelper; then, restart the server
				std::string filePath = Rain::getFullPathStr((*ccParam.config)["prod-root-dir"] + header[curFile].first);
				if (filePath != thisPath) {
					Rain::fastOutputFile(filePath,
										 reqAcc.substr(0, curFileReqBlock), true);
				} else {
					Rain::fastOutputFile(filePath + (*ccParam.config)["upload-tmp-app"],
										 reqAcc.substr(0, curFileReqBlock), true);
					delayExeWrite = true;
				}
				curFileDone += curFileReqBlock;
				if (curFileDone == header[curFile].second) {
					curFile++;
					curFileDone = 0;
					if (curFile == header.size()) { //prod-upload request is done
						//if the exe rewrite is delayed, we need to launch CRH helper now and terminate the program
						if (delayExeWrite) {
							Rain::sendBlockText(*ltrfdParam.cSocket, "Production upload request complete. Updating server requires server restart. Restarting now. Please reconnect to the server in a bit (a few seconds).");

							std::string crhelperAbspath = Rain::getFullPathStr((*ccParam.config)["crhelper"]),
								crhWorkingDir = Rain::pathToDir(crhelperAbspath),
								//"source" "destination" "additional commands to pass to restart"
								crhCmdLine = "\"" + thisPath + (*ccParam.config)["upload-tmp-app"] +
								"\" \"" + thisPath + "\"" +
								"prod-upload-success";
							crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
							ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);

							//terminate the program
							SetEvent(ccParam.hInputExitEvent);

							return 1;
						}

						reqAcc = "";
						headerLen = 0;
						curFileDone = 0;
						header.clear();
						curFile = 0;
						delayExeWrite = false;
					}
				}
			}

			return 0;
		}
		int HRProdDownload(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam) {
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ltrfdParam.delegateParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);
			if (!cdParam.authenticated) {
				Rain::sendBlockText(*ltrfdParam.cSocket, "Not yet authenticated.");
				return 0;
			}

			//request should be empty
			//send the files on the server production
			std::vector<std::string> files;
			Rain::getRelFilePathRec((*ccParam.config)["prod-root-dir"], files);

			//send header as one block, then block the files based on a block-size limit
			std::string response = Rain::tToStr(files.size()) + "\r\n";
			for (int a = 0; a < files.size(); a++)
				response += files[a] + "\r\n" + Rain::tToStr(Rain::getFileSize((*ccParam.config)["prod-root-dir"] + files[a])) + "\r\n";
			response += "\r\n";
			Rain::sendBlockTextRef(*ltrfdParam.cSocket, response);

			std::size_t blockMax = Rain::strToT<std::size_t>((*ccParam.config)["down-blocklen"]);
			for (int a = 0; a < files.size(); a++) {
				std::ifstream fileIn(files[a], std::ios::binary);
				std::size_t fileSize = Rain::getFileSize(files[a]);
				char *buffer = new char[blockMax];
				for (std::size_t b = 0; b < fileSize; b += blockMax) {
					fileIn.read(buffer, min(blockMax, fileSize - b));
					Rain::sendBlockText(*ltrfdParam.cSocket, static_cast<std::string>(buffer));
				}
				fileIn.close();
			}

			return 0;
		}
		int HRProdStop(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam) {
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ltrfdParam.delegateParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);
			if (!cdParam.authenticated) {
				Rain::sendBlockText(*ltrfdParam.cSocket, "Not yet authenticated.");
				return 0;
			}

			bool success = true;
			for (int a = 0; a < ccParam.serverStatus.size(); a++) {
				if (ccParam.serverStatus[a].status == "running") {
					static std::string exitCode = "exit";
					static DWORD dwWritten;
					static BOOL bSuccess;
					bSuccess = WriteFile(ccParam.serverStatus[a].stdInWr, exitCode.c_str(), static_cast<DWORD>(exitCode.length()), &dwWritten, NULL);
					CloseHandle(ccParam.serverStatus[a].stdInWr);
					if (!bSuccess) { //something went wrong while piping input; this is pretty bad
						Rain::reportError(GetLastError(), "Error while piping 'exit' to servers.");
						success = false;
					} else {
						//wait for server handle to exit with a timeout
						WaitForSingleObject(ccParam.serverStatus[a].process, Rain::strToT<DWORD>((*ccParam.config)["server-exit-to"]));

						ccParam.serverStatus[a].status = "stopped";
						CloseHandle(ccParam.serverStatus[a].stdInRd);
						CloseHandle(ccParam.serverStatus[a].stdOutRd);
						CloseHandle(ccParam.serverStatus[a].stdOutWr);
						CloseHandle(ccParam.serverStatus[a].process);
					}
				}
			}

			if (success) {
				Rain::sendBlockText(*ltrfdParam.cSocket, "All servers successfully stopped.");
			} else {
				Rain::sendBlockText(*ltrfdParam.cSocket, "Error while stopping servers.");
			}

			return 0;
		}
		int HRProdStart(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam) {
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ltrfdParam.delegateParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);
			if (!cdParam.authenticated) {
				Rain::sendBlockText(*ltrfdParam.cSocket, "Not yet authenticated.");
				return 0;
			}

			bool success = true;
			for (int a = 0; a < ccParam.serverStatus.size(); a++) {
				if (ccParam.serverStatus[a].status == "stopped") {
					//run the server as a child process with pipes
					//everything else should be like a normal run
					SECURITY_ATTRIBUTES sa;
					sa.nLength = sizeof(SECURITY_ATTRIBUTES);
					sa.bInheritHandle = TRUE;
					sa.lpSecurityDescriptor = NULL;
					HANDLE g_hChildStd_OUT_Rd = NULL,
						g_hChildStd_OUT_Wr = NULL,
						g_hChildStd_IN_Rd = NULL,
						g_hChildStd_IN_Wr = NULL;
					if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0) ||
						!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) ||
						!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &sa, 0) ||
						!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
						Rain::reportError(GetLastError(), "Error while creating pipes for server at " + ccParam.serverStatus[a].path);
						success = false;
						continue;
					}

					STARTUPINFO sinfo;
					PROCESS_INFORMATION pinfo;
					ZeroMemory(&sinfo, sizeof(sinfo));
					ZeroMemory(&pinfo, sizeof(pinfo));
					sinfo.cb = sizeof(sinfo);
					sinfo.dwFlags |= STARTF_USESTDHANDLES;
					sinfo.hStdOutput = g_hChildStd_OUT_Wr;
					sinfo.hStdInput = g_hChildStd_IN_Rd;
					if (!CreateProcess(
						ccParam.serverStatus[a].path.c_str(),
						NULL,
						NULL,
						NULL,
						TRUE,
						CREATE_NEW_CONSOLE,
						NULL,
						Rain::getPathDirectory(ccParam.serverStatus[a].path).c_str(),
						&sinfo,
						&pinfo)) { //try to fail peacefully
						Rain::reportError(GetLastError(), "Error while starting server at " + ccParam.serverStatus[a].path);
						success = false;
						continue;
					}

					ccParam.serverStatus[a].process = pinfo.hProcess;
					ccParam.serverStatus[a].stdInRd = g_hChildStd_IN_Rd;
					ccParam.serverStatus[a].stdInWr = g_hChildStd_IN_Wr;
					ccParam.serverStatus[a].stdOutRd = g_hChildStd_OUT_Rd;
					ccParam.serverStatus[a].stdOutWr = g_hChildStd_OUT_Wr;

					ccParam.serverStatus[a].status = "running";
				}
			}

			if (success) {
				Rain::sendBlockText(*ltrfdParam.cSocket, "All servers successfully started.");
			} else {
				Rain::sendBlockText(*ltrfdParam.cSocket, "Error while starting servers.");
			}

			return 0;
		}
		int HRSyncStop(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam) {
			return 0;
		}
		int HRSyncStart(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam) {
			return 0;
		}
	}
}