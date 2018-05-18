#include "RequestHandlers.h"

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		int HandleRequest(ConnectionDelegateParam &cdParam) {
			static const std::map<std::string, RequestMethodHandler> methodHandlerMap{
				{"authenticate", HRAuthenticate}, //validates a socket connection session
				{"prod-upload", HRProdUpload}, //updates server-side production files with client files
				{"prod-download", HRProdDownload}, //request for current production files to client
				{"prod-stop", HRProdStop}, //stop all server services
				{"prod-start", HRProdStart}, //start all server services
				{"sync-stop", HRSyncStop}, //stops constant updates and enables other commands
				{"sync-start", HRSyncStart}, //request constant updates for changed production files; disables other commands except sync-stop
			};

			cdParam.requestMethod = cdParam.request.substr(0, cdParam.request.find(' '));
			cdParam.request = cdParam.request.substr(cdParam.request.find(' ') + 1, cdParam.request.length());

			auto handler = methodHandlerMap.find(cdParam.requestMethod);
			int handlerRet = 0;
			if (handler != methodHandlerMap.end())
				handlerRet = handler->second(cdParam);

			//clear request on exit
			cdParam.request = "";

			return handlerRet;
		}

		int HRAuthenticate(ConnectionDelegateParam &cdParam) {
			if (cdParam.authenticated) {
				Rain::sendBlockText(*cdParam.cSocket, "Already authenticated.");
			} else if (cdParam.config->at("client-auth-pass") != cdParam.request) {
				Rain::sendBlockText(*cdParam.cSocket, "Authentication failed.");
			} else {
				Rain::sendBlockText(*cdParam.cSocket, "Authentication success.");
				cdParam.authenticated = true;
			}

			return 0;
		}
		int HRProdUpload(ConnectionDelegateParam &cdParam) {
			if (!cdParam.authenticated) {
				Rain::sendBlockText(*cdParam.cSocket, "Not yet authenticated.");
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
					}
					for (int a = 0; a < n; a++) {
						static std::string tmp;
						std::getline(headerSS, tmp, '\n'); //should extract up until and including \n
						header[a].second = Rain::strToT<std::size_t>(tmp);
					}

					//nuke prod (which should work, except for the current executable)
					Rain::recursiveRmDir((*cdParam.config)["prod-root-dir"]);
				}
			}

			//if the header is processed, append requests to correct files
			if (headerLen != 0) {
				reqAcc += cdParam.request;
				std::size_t curFileReqBlock = min(reqAcc.length(), header[curFile].second - curFileDone);

				//as long as the file is modifiable (i.e. it's not the current exe), update it
				//otherwise, save the data in a temp file, and mark a flag to delay the copying of the temp file to the exe file with CRHelper; then, restart the server
				std::string filePath = Rain::getFullPathStr((*cdParam.config)["prod-root-dir"] + header[curFile].first);
				if (filePath != thisPath) {
					Rain::fastOutputFile(filePath,
										 reqAcc.substr(0, curFileReqBlock), true);
				} else {
					Rain::fastOutputFile(filePath + (*cdParam.config)["upload-tmp-app"],
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
							Rain::sendBlockText(*cdParam.cSocket, "Production upload request complete. Updating server requires server restart. Restarting now. Please reconnect to the server in a bit (a few seconds).");

							std::string crhelperAbspath = Rain::getFullPathStr((*cdParam.config)["crhelper"]),
								crhWorkingDir = Rain::pathToDir(crhelperAbspath),
								//"source" "destination" "additional commands to pass to restart"
								crhCmdLine = "\"" + thisPath + (*cdParam.config)["upload-tmp-app"] +
								"\" \"" + thisPath + "\"" +
								"prod-upload-success";
							crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
							ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);

							//terminate the program

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
		int HRProdDownload(ConnectionDelegateParam &cdParam) {
			if (!cdParam.authenticated) {
				Rain::sendBlockText(*cdParam.cSocket, "Not yet authenticated.");
				return 0;
			}

			//send the files on the server
		}
		int HRProdStop(ConnectionDelegateParam &cdParam) {

		}
		int HRProdStart(ConnectionDelegateParam &cdParam) {

		}
		int HRSyncStop(ConnectionDelegateParam &cdParam) {

		}
		int HRSyncStart(ConnectionDelegateParam &cdParam) {

		}
	}
}