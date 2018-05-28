#include "RequestHandlers.h"

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		int HandleRequest(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			static const std::map<std::string, RequestMethodHandler> methodHandlerMap{
				{"authenticate", HRAuthenticate}, //validates a socket connection session
				{"prod-upload", HRProdUpload}, //updates server-side production files with client files
				{"prod-download", HRProdDownload}, //request for current production files to client
				{"prod-stop", HRProdStop}, //stop all server services
				{"prod-start", HRProdStart}, //start all server services
				{"sync-stop", HRSyncStop}, //stops constant updates and enables other commands
				{"sync-start", HRSyncStart}, //request constant updates for changed production files; disables other commands except sync-stop
			};
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			//if we are in a multi-block request, send straight to handler
			if (chParam.persistentRequestMethod != "") {
				chParam.requestMethod = chParam.persistentRequestMethod;
			} else {
				//takes until the end of string if ' ' can't be found
				size_t firstSpace = chParam.request.find(' ');
				chParam.requestMethod = chParam.request.substr(0, firstSpace);

				if (firstSpace != chParam.request.npos)
					chParam.request = chParam.request.substr(chParam.request.find(' ') + 1, chParam.request.length());
				else
					chParam.request = "";
			}

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
			if (chParam.request == "success" || chParam.request == "auth-done")
				chParam.lastSuccess = true;
			else if (chParam.request == "fail")
				chParam.lastSuccess = false;
			chParam.waitingRequests--;
			return 0;
		}
		int HRProdUpload(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			return 0;
		}
		int HRProdDownload(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			if (chParam.request == "file-read-error" || chParam.request == "auth-error")
				chParam.lastSuccess = false;
			else { //success, so start replacing files in prod, which should not have any problems
				chParam.persistentRequestMethod = "prod-download";
				//clear these variables on request end
				//unprocessed request buffer, since request might be blocked
				static std::string reqAcc = "",
					//path to the current exe, which might cause problems when updating
					thisPath = Rain::pathToAbsolute(Rain::getExePath());
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
					std::size_t emptyLine = Rain::rabinKarpMatch(chParam.request, "\r\n\r\n");
					reqAcc += chParam.request;
					chParam.request = "";
					if (emptyLine != -1) {
						headerLen = reqAcc.length() + emptyLine + 1; //+1 because \r\n is two characters

						//process header
						std::stringstream headerSS;
						headerSS << reqAcc << chParam.request.substr(0, emptyLine);
						reqAcc = chParam.request.substr(emptyLine + 2, chParam.request.length());

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
						Rain::recursiveRmDir((*chParam.config)["prod-root-dir"]);
					}
				}

				//if the header is processed, append requests to correct files
				if (headerLen != 0) {
					reqAcc += chParam.request;
					std::size_t curFileReqBlock = min(reqAcc.length(), header[curFile].second - curFileDone);

					//as long as the file is modifiable (i.e. it's not the current exe), update it
					//otherwise, save the data in a temp file, and mark a flag to delay the copying of the temp file to the exe file with CRHelper; then, restart the server
					std::string filePath = Rain::pathToAbsolute((*chParam.config)["prod-root-dir"] + header[curFile].first);
					if (filePath != thisPath) {
						Rain::printToFile(filePath,
										  reqAcc.substr(0, curFileReqBlock), true);
					} else {
						Rain::printToFile(filePath + (*chParam.config)["upload-tmp-app"],
										  reqAcc.substr(0, curFileReqBlock), true);
						delayExeWrite = true;
					}
					curFileDone += curFileReqBlock;
					if (curFileDone == header[curFile].second) {
						curFile++;
						curFileDone = 0;
						if (curFile == header.size()) { //prod-upload request is done
							reqAcc = "";
							headerLen = 0;
							curFileDone = 0;
							header.clear();
							curFile = 0;
							delayExeWrite = false;

							//set success flags and reset things
							chParam.persistentRequestMethod = "";
							chParam.lastSuccess = true;
							chParam.waitingRequests--;
						}
					}
				}
			}
			return 0;
		}
		int HRProdStop(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			if (chParam.request == "fail" || chParam.request == "auth-error")
				chParam.lastSuccess = false;
			else
				chParam.lastSuccess = true;
			chParam.waitingRequests--;
			return 0;
		}
		int HRProdStart(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			return 0;
		}
		int HRSyncStop(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			return 0;
		}
		int HRSyncStart(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			return 0;
		}
	}
}