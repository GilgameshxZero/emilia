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
				Rain::tsCout("Info: Received request from update client: ", cdParam.requestMethod, ".\r\n");
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

		/*
		int HRProdUpload(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			if (!cdParam.authenticated) {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-upload auth-error");
				Rain::tsCout("Failure: 'prod-upload' fail; not yet authenticated.\r\n");
				fflush(stdout);
				return 0;
			}

			//assume servers are shut down here

			//test this to see if request is a normal message or additional filedata
			static bool receivingFiledata = false;

			//flag for if we need to use CRH
			static bool delayExeWrite = false;

			//path to the current exe
			static std::string thisPath = Rain::pathToAbsolute(Rain::getExePath());

			if (!receivingFiledata &&
				cdParam.request == "file-read-error") {
				//if there is a problem before the remote client has started transferring files, abort
				Rain::tsCout("Failure: Client uncountered 'file-read-error' while attempting to upload files to production. Aborting...\r\n");
				fflush(stdout);
			} else if (!receivingFiledata &&
					   cdParam.request == "finish-success") {
				//if we are here, then the client has finished transferring files, and has indicated success
				//IMPORTANT: check if there were problems overwriting the current exe (which there should be). If so, start up the CRH
				if (delayExeWrite) {
					//send success code now, before server restarts, and wait on a restart okay receipt
					Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-upload delay");
					Rain::tsCout("Info: 'prod-upload' needs to modify the current .exe. This program will exit, start the CRH, which will finish the upload process and restart this program. Please wait a moment...\r\n");
					fflush(stdout);
				} else {
					Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-upload success");
					Rain::tsCout("Success: 'prod-upload' success.\r\n");
					fflush(stdout);
				}
			} else if (!receivingFiledata &&
					   cdParam.request == "restart-okay") {
				std::string crhelperAbspath = Rain::pathToAbsolute((*ccParam.config)["crhelper"]),
					crhWorkingDir = Rain::getPathDir(crhelperAbspath),
					//"source" "destination" "additional commands to pass to restart"
					crhCmdLine = "\"" + thisPath + (*ccParam.config)["update-tmp-ext"] +
					"\" \"" + thisPath + "\" " +
					"prod-upload-success";
				crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
				ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);

				//terminate the program and the connection, to be resurrected by CRH
				SetEvent(ccParam.hInputExitEvent);
				return 1;
			} else if (!receivingFiledata &&
					   cdParam.request == "start") {
				//the server has indicated to start transferring files, so set the persistent request method so that we know that all data from now on is filedata
				receivingFiledata = true;
			} else if (receivingFiledata) {
				//receiving files from client

				//clear these variables on filedata receive end
				//unprocessed request buffer, since request might be blocked
				static std::string reqAcc = "";
				//position of end of the header block
				static std::size_t headerLen = 0,
					//how many bytes of current file we have on disk already
					curFileDone = 0;
				//ordered list of files
				static std::vector<std::pair<std::string, std::size_t>> header;
				//file currently processing
				static int curFile = 0;

				if (headerLen == 0) {
					std::size_t emptyLine = Rain::rabinKarpMatch(cdParam.request, "\r\n\r\n");
					if (emptyLine != -1) {
						headerLen = reqAcc.length() + emptyLine + 1; //+1 because \r\n is two characters

																	 //process header
						std::stringstream headerSS;
						headerSS << reqAcc << cdParam.request.substr(0, emptyLine);
						reqAcc = cdParam.request.substr(emptyLine + 4, cdParam.request.length());

						int n;
						headerSS >> n;
						header.resize(n);
						std::string tmp;
						std::getline(headerSS, tmp); //ignore the newline
						for (int a = 0; a < n; a++) {
							std::getline(headerSS, header[a].first, '\n'); //should extract up until and including \n
							Rain::strTrimWhite(&header[a].first); //clean up \r
							static std::string tmp;
							std::getline(headerSS, tmp, '\n'); //should extract up until and including \n
							header[a].second = Rain::strToT<std::size_t>(tmp);
						}

						//nuke prod (which should work, except for the current executable)
						Rain::rmDirRec((*ccParam.config)["prod-root-dir"]);
					} else {
						reqAcc += cdParam.request;
					}
					cdParam.request = "";
				}

				//if the header is processed, append requests to correct files
				if (headerLen != 0) {
					reqAcc += cdParam.request;

					while (header.size() == 0 ||
						   header[curFile].second == 0 ||
						   reqAcc.length() > 0) {
						//care for edge cases
						if (header.size() != 0) {
							std::size_t curFileReqBlock = min(reqAcc.length(), header[curFile].second - curFileDone);

							//as long as the file is modifiable (i.e. it's not the current exe), update it
							//otherwise, save the data in a temp file, and mark a flag to delay the copying of the temp file to the exe file with CRHelper; then, restart the server
							std::string filePath = Rain::pathToAbsolute((*ccParam.config)["prod-root-dir"] + header[curFile].first);
							if (filePath != thisPath) {
								Rain::createDirRec(Rain::getPathDir(filePath));
								Rain::printToFile(filePath,
												  reqAcc.substr(0, curFileReqBlock), true);
							} else {
								Rain::printToFile(filePath + (*ccParam.config)["update-tmp-ext"],
												  reqAcc.substr(0, curFileReqBlock), true);
								delayExeWrite = true;
							}
							reqAcc = reqAcc.substr(curFileReqBlock, reqAcc.length());

							curFileDone += curFileReqBlock;
							if (curFileDone == header[curFile].second) {
								curFile++;
								curFileDone = 0;
							}
						}

						//test if filedata transfer is complete
						if (curFile == header.size()) {
							//filedata is done
							receivingFiledata = false;

							//reset statics
							reqAcc = "";
							headerLen = 0;
							curFileDone = 0;
							header.clear();
							curFile = 0;

							//exit function and wait for another prod-download block with success
							break;
						}
					}
				}
			}

			return 0;
		}
		int HRProdDownload(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			if (!cdParam.authenticated) {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-download auth-error");
				Rain::tsCout("Failure: 'prod-download' fail; not yet authenticated.\r\n");
				fflush(stdout);
				return 0;
			}

			//request should be empty
			if (cdParam.request != "") {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-download request-error");
				Rain::tsCout("Failure: 'prod-download' fail; request from client in wrong format.\r\n");
				fflush(stdout);
				return 0;
			}

			//send over the files on server production, possibly in multiple blocks
			std::vector<std::string> files;
			files = Rain::getFilesRec((*ccParam.config)["prod-root-dir"]);

			//send header as one block, then block the files based on a block-size limit
			std::string response = "prod-download " + Rain::tToStr(files.size()) + "\r\n";
			bool filesReadable = true;
			for (int a = 0; a < files.size(); a++) {
				std::size_t fileSize = Rain::getFileSize((*ccParam.config)["prod-root-dir"] + files[a]);
				if (fileSize == std::size_t(-1)) { //error
					filesReadable = false;
					break;
				}
				response += files[a] + "\r\n" + Rain::tToStr(fileSize) + "\r\n";
			}
			if (!filesReadable) {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-download file-read-error");
				Rain::tsCout("Failure: 'prod-download' fail; some files were unreadable.\r\n");
				fflush(stdout);
				return 0;
			}

			//indicate we are starting the upload
			Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-download start");

			response += "\r\n";
			Rain::sendBlockMessage(*ssmdhParam.ssm, &response);

			std::size_t blockMax = Rain::strToT<std::size_t>((*ccParam.config)["transfer-blocklen"]);
			for (int a = 0; a < files.size(); a++) {
				std::ifstream fileIn((*ccParam.config)["prod-root-dir"] + files[a], std::ios::binary);
				std::size_t fileSize = Rain::getFileSize((*ccParam.config)["prod-root-dir"] + files[a]);
				char *buffer = new char[blockMax];
				for (std::size_t b = 0; b < fileSize; b += blockMax) {
					fileIn.read(buffer, min(blockMax, fileSize - b));

					//send this buffer as data, with the prod-upload methodname, all in a single block
					Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-download " + std::string(buffer, min(blockMax, fileSize - b)));
				}
				fileIn.close();
			}

			//indicate we are done with the file upload
			Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-download finish-success");
			Rain::tsCout("Success: 'prod-download' completed.\r\n");
			fflush(stdout);

			return 0;
		}
		int HRProdStop(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			if (!cdParam.authenticated) {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-stop auth-error");
				Rain::tsCout("Failure: 'prod-stop' fail; not yet authenticated.\r\n");
				fflush(stdout);
				return 0;
			}

			bool success = true;
			for (int a = 0; a < ccParam.serverStatus.size(); a++) {
				if (ccParam.serverStatus[a].status == "running") {
					static std::string exitCode = "exit\r\n";
					static DWORD dwWritten;
					static BOOL bSuccess;
					bSuccess = WriteFile(ccParam.serverStatus[a].stdInWr, exitCode.c_str(), static_cast<DWORD>(exitCode.length()), &dwWritten, NULL);
					CloseHandle(ccParam.serverStatus[a].stdInWr);
					if (!bSuccess) { //something went wrong while piping input; this is pretty bad
						Rain::reportError(GetLastError(), "Error while piping 'exit' to servers.");
						success = false;
					} else {
						//wait for server handle to exit with a timeout
						WaitForSingleObject(ccParam.serverStatus[a].process, Rain::strToT<DWORD>((*ccParam.config)["update-server-exit-to"]));

						ccParam.serverStatus[a].status = "stopped";
						CloseHandle(ccParam.serverStatus[a].stdInRd);
						CloseHandle(ccParam.serverStatus[a].stdOutRd);
						CloseHandle(ccParam.serverStatus[a].stdOutWr);
						CloseHandle(ccParam.serverStatus[a].process);
					}
				}
			}

			if (success) {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-stop success");
				Rain::tsCout("Success: 'prod-stop' completed.\r\n");
				fflush(stdout);
			} else {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-stop fail");
				Rain::tsCout("Failure: 'prod-stop' failed; some servers were not stopped.\r\n");
				fflush(stdout);
			}

			return 0;
		}
		int HRProdStart(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			if (!cdParam.authenticated) {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-start auth-error");
				Rain::tsCout("Failure: 'prod-start' fail; not yet authenticated.\r\n");
				fflush(stdout);
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

					//don't pipe stdout, so that server output is still displayed in console
					//sinfo.hStdOutput = g_hChildStd_OUT_Wr;
					sinfo.hStdOutput = NULL;

					//used to tell server to exit
					sinfo.hStdInput = g_hChildStd_IN_Rd;

					if (!CreateProcess(
						ccParam.serverStatus[a].path.c_str(),
						NULL,
						NULL,
						NULL,
						TRUE,
						CREATE_NEW_CONSOLE,
						NULL,
						Rain::getPathDir(ccParam.serverStatus[a].path).c_str(),
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
				Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-start success");
				Rain::tsCout("Success: 'prod-start' completed.\r\n");
				fflush(stdout);
			} else {
				Rain::sendBlockMessage(*ssmdhParam.ssm, "prod-start fail");
				Rain::tsCout("Failure: 'prod-start' failed; some servers were not started.\r\n");
				fflush(stdout);
			}

			return 0;
		}
		int HRSyncStop(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			return 0;
		}
		int HRSyncStart(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			return 0;
		}*/
	}
}