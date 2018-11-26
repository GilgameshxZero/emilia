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

			return 0;
		}

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
			if (chParam.request == "success" || chParam.request == "auth-done")
				chParam.lastSuccess = 0;
			else if (chParam.request == "fail")
				chParam.lastSuccess = 1;
			else
				chParam.lastSuccess = -1;
			chParam.waitingRequests--;
			return 0;
		}
		int HRProdUpload(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			if (chParam.request == "success")
				chParam.lastSuccess = 0;
			else if (chParam.request == "delay")
				chParam.lastSuccess = 1;
			else
				chParam.lastSuccess = -1;
			chParam.waitingRequests--;
			return 0;
		}
		int HRProdDownload(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);

			//test this to see if request is a normal message or additional filedata
			static bool receivingFiledata = false;

			//if there is a problem before the remote server has started transferring files
			if (receivingFiledata == false &&
				(chParam.request == "file-read-error" || chParam.request == "auth-error"))
				chParam.lastSuccess = 1;
			else if (receivingFiledata == false &&
					 chParam.request == "finish-success") {
				//if we are here, then the server has finished transferring files, and has indicated success
				chParam.lastSuccess = 0;
				chParam.waitingRequests--;
			} else if (receivingFiledata == false &&
					   chParam.request == "start") {
				//the server has indicated to start transferring files, so set the persistent request method so that we know that all data from now on is filedata
				receivingFiledata = true;
			} else if (receivingFiledata == true) {
				//clear these variables on filedata receive end
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

				if (headerLen == 0) {
					std::size_t emptyLine = Rain::rabinKarpMatch(chParam.request, "\r\n\r\n");
					if (emptyLine != -1) {
						headerLen = reqAcc.length() + emptyLine + 1; //+1 because \r\n is two characters

																	 //process header
						std::stringstream headerSS;
						headerSS << reqAcc << chParam.request.substr(0, emptyLine);
						reqAcc = chParam.request.substr(emptyLine + 4, chParam.request.length());

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
						Rain::rmDirRec((*chParam.config)["prod-root-dir"]);

						Rain::tsCout("Info: Header received: ", n, " files to be downloaded.\r\n");
						fflush(stdout);
					} else {
						reqAcc += chParam.request;
					}
					chParam.request = "";
				}

				//if the header is processed, append requests to correct files
				if (headerLen != 0) {
					reqAcc += chParam.request;

					while (header.size() == 0 ||
						   header[curFile].second == 0 ||
						   reqAcc.length() > 0) {
						//care for edge cases
						if (header.size() != 0) {
							Rain::tsCout("Info: Receiving file ", curFile + 1, " of ", header.size(), ". Done: ", curFileDone, " of ", header[curFile].second, ".\r\n");
							fflush(stdout);

							std::size_t curFileReqBlock = min(reqAcc.length(), header[curFile].second - curFileDone);

							//all files in local production should be modifiable
							std::string filePath = Rain::pathToAbsolute((*chParam.config)["prod-root-dir"] + header[curFile].first);
							Rain::createDirRec(Rain::getPathDir(filePath));
							Rain::printToFile(filePath, reqAcc.substr(0, curFileReqBlock), true);
							reqAcc = reqAcc.substr(curFileReqBlock, reqAcc.length());

							curFileDone += curFileReqBlock;
							if (curFileDone == header[curFile].second) {
								curFile++;
								curFileDone = 0;
								Rain::tsCout("Info: File ", curFile, " of ", header.size(), " complete.\r\n");
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
		int HRProdStop(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			if (chParam.request == "success")
				chParam.lastSuccess = 0;
			else if (chParam.request == "fail" || chParam.request == "auth-error")
				chParam.lastSuccess = 1;
			else
				chParam.lastSuccess = -1;
			chParam.waitingRequests--;
			return 0;
		}
		int HRProdStart(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ConnectionHandlerParam &chParam = *reinterpret_cast<ConnectionHandlerParam *>(csmdhParam.delegateParam);
			if (chParam.request == "success")
				chParam.lastSuccess = 0;
			else if (chParam.request == "fail" || chParam.request == "auth-error")
				chParam.lastSuccess = 1;
			else
				chParam.lastSuccess = -1;
			chParam.waitingRequests--;
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