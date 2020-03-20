#include "http-server.hpp"

namespace Emilia {
	namespace HTTPServer {
		int onConnect(void *funcParam) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			//logging
			Rain::tsCout("[HTTP] [", *ssmdhParam.cSocket, "] [", ++ccParam.connectedClients, "] Connected ", Rain::getClientNumIP(*ssmdhParam.cSocket), ".", Rain::CRLF);

			//create the delegate parameter for the first time
			ConnectionDelegateParam *cdParam = new ConnectionDelegateParam();
			ssmdhParam.delegateParam = reinterpret_cast<void *>(cdParam);

			//delay all possible initialization of rtParam to here
			cdParam->request = "";
			cdParam->requestMethod = "";
			cdParam->contentLength = -1;
			cdParam->headerBlockLength = -1;

			//thread to process messages in parallel
			cdParam->newMessageEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			cdParam->requestThread = std::thread(requestThreadFunc, std::ref(ssmdhParam));

			//other things
			cdParam->fileBufLen = (*ccParam.config)["emilia-buffer"].i();
			cdParam->buffer = new char[cdParam->fileBufLen];

			return 0;
		}
		int onMessage(void *funcParam) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			//accumulate messages until a complete request is found
			//for now, only accept GET requests and non-chunked POST requests (those with a content-length header)
			//of course, for POST requests, we will need to accumulate not just the header but also the body of the request
			//onMessage is also responsible for parsing the header before passing it on to processRequest

			//trigger the other thread while doing this
			cdParam.requestModifyMutex.lock();
			cdParam.request += *ssmdhParam.message;
			cdParam.requestModifyMutex.unlock();
			SetEvent(cdParam.newMessageEvent);

			return 0;
		}
		int onDisconnect(void *funcParam) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			//join the message thread
			cdParam.disconnectStarted = true;
			SetEvent(cdParam.newMessageEvent);
			CancelSynchronousIo(cdParam.requestThread.native_handle());
			cdParam.requestThread.join();

			//free the delegate parameter
			delete cdParam.buffer;
			delete &cdParam;

			//logging
			Rain::tsCout("[HTTP] [", *ssmdhParam.cSocket, "] [", --ccParam.connectedClients, "] Disconnected ", Rain::getClientNumIP(*ssmdhParam.cSocket), ".", Rain::CRLF);
			std::cout.flush();

			return 0;
		}

		int requestThreadFunc(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			static const std::string headerDelim = Rain::CRLF + Rain::CRLF;

			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			while (true) {
				//wait for new messages to be added to the request from onMessage
				WaitForSingleObject(cdParam.newMessageEvent, INFINITE);

				//check if socket disconnected; if so, terminate thread
				if (cdParam.disconnectStarted) {
					break;
				}

				//if we don't know the requestMethod yet, attempt to calculate it now
				if (cdParam.requestMethod == "") {
					std::size_t firstSpace = cdParam.request.find(' ');

					if (firstSpace != std::string::npos)  //if there's a space, then everything before that is the requestMethod
						cdParam.requestMethod = cdParam.request.substr(0, firstSpace);
				}

				//determine whether the request is complete based on the requestMethod and the currently received request
				if (cdParam.requestMethod == "") {  //still don't know the request method yet, so we should wait for more messages to come in
					continue;
				} else if (cdParam.requestMethod == "GET") {                                       //GET requests complete when the last four characters are "\r\n\r\n"
					if (cdParam.request.substr(cdParam.request.length() - 4, 4) == headerDelim) {  //get ready to process the request
						cdParam.contentLength = 0;
						cdParam.headerBlockLength = cdParam.request.length() - headerDelim.length();
					} else  //if the last line isn't blank, keep on waiting for more messages
						continue;
				} else if (cdParam.requestMethod == "POST") {  //identify the header block, find the 'content-length' header, and only process the request when the body is of the correct length
					cdParam.headerBlockLength = Rain::rabinKarpMatch(cdParam.request, headerDelim);
					if (cdParam.headerBlockLength == -1)  //still waiting on headers
						continue;

					//if don't know contentLength yet from previous times we get here, try again to get it
					if (cdParam.contentLength == -1) {
						//a lot of these parameters are not used, but that's fine, since it's a one time cost
						std::map<std::string, std::string> headers;
						std::string headerBlock = cdParam.request.substr(0, cdParam.headerBlockLength);
						std::stringstream ss;
						std::string requestURI, httpVersion;

						ss << headerBlock;
						ss >> cdParam.requestMethod >> requestURI >> httpVersion;
						parseHeaders(ss, headers);
						cdParam.contentLength = Rain::strToT<std::size_t>(headers["content-length"]);
					}

					//accumulate request until body is contentLength long
					unsigned long long requestLength = cdParam.headerBlockLength + headerDelim.length() + cdParam.contentLength;
					if (cdParam.request.length() < requestLength)
						continue;
					else if (cdParam.request.length() > requestLength) {//for some reason, the request is longer than anticipated, so terminate the socket
						shutdown(*ssmdhParam.cSocket, SD_BOTH);
						break;
					}
				}

				//at this point, we have a complete request and we know the size of the header block
				//so, parse the header and log the request, then send it over to processRequest
				std::map<std::string, std::string> headers;
				std::string headerBlock = cdParam.request.substr(0, cdParam.headerBlockLength),
					bodyBlock = cdParam.request.substr(cdParam.headerBlockLength + headerDelim.length(), cdParam.contentLength);  //the body is everything after the header deliminater
				std::stringstream ss;
				std::string requestURI, httpVersion;

				ss << headerBlock;
				ss >> cdParam.requestMethod >> requestURI >> httpVersion;
				parseHeaders(ss, headers);

				//log the request manually so that we don't log responses
				Rain::tsCout("[HTTP] [", *ssmdhParam.cSocket, "] ", cdParam.requestMethod, " ", requestURI, Rain::CRLF);
				std::cout.flush();
				ccParam.logHTTP->logString(&cdParam.request);

				//reset parameters
				cdParam.requestModifyMutex.lock();
				cdParam.request = "";
				cdParam.requestModifyMutex.unlock();

				//send the parsed headers and bodyBlock and other parameters over to processRequest
				int prRet = processRequest(ssmdhParam,
					requestURI,
					httpVersion,
					headers,
					bodyBlock);

				//reset parameters
				cdParam.requestMethod = "";
				cdParam.contentLength = -1;
				cdParam.headerBlockLength = -1;

				//prRet: < 0 is error, 0 is keep-alive, and > 0 is peacefully close
				if (prRet != 0) {
					shutdown(*ssmdhParam.cSocket, SD_BOTH);
					break;
				}
			}

			return 0;
		}

		int processRequest(Rain::ServerSocketManager::DelegateHandlerParam
			&ssmdhParam,
			std::string &requestURI,
			std::string &httpVersion,
			std::map<std::string, std::string> &headers,
			std::string &bodyBlock) {
			//a complete, partially parsed, request has come in; see onMessage for the methods that we process and those that we don't
			//we can return 0 to keep socket open, and nonzero to close the socket
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			Rain::Configuration &config = *ccParam.config;
			SOCKET &cSocket = *ssmdhParam.cSocket;

			//only accept HTTP version 1.1
			if (httpVersion != "HTTP/1.1")
				return 1;

			//decompose the requestURI
			std::size_t questionMarkPos = requestURI.find("?"),
				hashtagPos = requestURI.find("#");
			std::string requestFilePath = requestURI.substr(0, questionMarkPos),
				requestQuery = "";
			if (questionMarkPos != std::string::npos)
				requestQuery = requestURI.substr(questionMarkPos + 1, hashtagPos);;

			//parse the URIs
			requestFilePath = Rain::strDecodeURI(requestFilePath);
			std::replace(requestFilePath.begin(), requestFilePath.end(), '/', '\\');

			//decompose requestFilePath and check if we need to substitute a default file
			std::string requestFile = Rain::getPathFile(requestFilePath),
				requestFileDir = Rain::getPathDir(requestFilePath);
			if (requestFile.length() == 0)
				requestFile = config["http-index"].s();

			//compose the final path
			requestFileDir = Rain::pathToAbsolute(ccParam.project + config["http-root"].s() + requestFileDir);
			requestFilePath = Rain::pathToAbsolute(requestFileDir + requestFile);

			//we are a cgi script if the current r has any of the requestFilePath cgiScript strings as a substring
			std::set<std::string> cgi = config["http-cgi"].keys();
			bool isCgiScript = false;
			for (auto it = cgi.begin(); it != cgi.end(); it++) {
				std::string thisPath = ccParam.project + config["http-root"].s() + *it;
				if (requestFilePath.substr(0, thisPath.length()) == thisPath) {
					isCgiScript = true;
					break;
				}
			}

			//response headers that we actually send back should be built on top of customHeaders, so copy customHeaders
			std::map<std::string, std::string> responseHeaders;
			std::string responseStatus, responseBody;
			std::set<std::string> customHeaders = config["http-headers"].keys();
			for (auto it = customHeaders.begin(); it != customHeaders.end(); it++) {
				responseHeaders.insert(std::make_pair(*it, config["http-headers"][*it].s()));
			}

			//assert that the requestFile exists; if not, send back prespecified 404 file
			//moreover, the path has to be under the server root path, unless its a cgi script
			//ensures that the server will not serve outside its root directory unless a cgi script is specified
			if (!Rain::fileExists(requestFilePath) ||
				(!Rain::isSubPath(ccParam.project + config["http-root"].s(), requestFilePath) &&
					cgi.find(requestFilePath) == cgi.end())) {
				requestFilePath = ccParam.project + config["http-404"].s();
			}
			if (isCgiScript) {
				//branch if the file is a cgi script
				//run the file at FilePath as a cgi script, and respond with its content
				//set the current environment block as well as additional environment parameters for the script
				std::string envBlock;
				envBlock += "QUERY_STRING=" + requestQuery;
				envBlock.push_back('\0');
				std::map<std::string, std::string>::iterator iterator = headers.find("referer");
				if (iterator != headers.end()) {
					envBlock += "HTTP_REFERER=" + iterator->second;
					envBlock.push_back('\0');
				}
				iterator = headers.find("content-length");
				if (iterator != headers.end()) {
					envBlock += "CONTENT_LENGTH=" + iterator->second;
					envBlock.push_back('\0');
				}
				iterator = headers.find("host");
				if (iterator != headers.end()) {
					envBlock += "HTTP_HOST=" + iterator->second;
					envBlock.push_back('\0');
				}
				iterator = headers.find("range");
				if (iterator != headers.end()) {
					envBlock += "HTTP_RANGE=" + iterator->second;
					envBlock.push_back('\0');
				}

				//also, store some other useful information as environment variables
				envBlock += "CLIENT_IP=" + Rain::getClientNumIP(cSocket);
				envBlock.push_back('\0');
				envBlock += "EMILIA_VERSION=" + getVersionStr();
				envBlock.push_back('\0');

				//append current environment block
				LPCH curEnvBlock = GetEnvironmentStrings();
				int prevVarBeg = -1;
				for (int a = 0;; a++) {
					if (curEnvBlock[a] == '\0') {
						envBlock += std::string(curEnvBlock + prevVarBeg + 1, curEnvBlock + a);
						envBlock.push_back('\0');
						prevVarBeg = a;
						if (curEnvBlock[a + 1] == '\0')
							break;
					}
				}
				FreeEnvironmentStrings(curEnvBlock);

				//end the environment variables
				envBlock.push_back('\0');

				//pipe output of script to a local buffer, and create an input pipe as well
				//this is all pretty technical windows stuff
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
					Rain::errorAndCout(GetLastError(), "Error while setting up pipe for cgi script " + requestFilePath);
					return -1;  //something went wrong, terminate socket, try to fail peacefully
				}

				//execute the script with the pipes and environment block
				STARTUPINFO sinfo;
				PROCESS_INFORMATION pinfo;
				ZeroMemory(&sinfo, sizeof(sinfo));
				ZeroMemory(&pinfo, sizeof(pinfo));
				sinfo.cb = sizeof(sinfo);
				sinfo.dwFlags |= STARTF_USESTDHANDLES;
				sinfo.hStdOutput = g_hChildStd_OUT_Wr;
				sinfo.hStdInput = g_hChildStd_IN_Rd;
				if (!CreateProcess(
					requestFilePath.c_str(),
					NULL,
					NULL,
					NULL,
					TRUE,
					CREATE_NEW_CONSOLE,
					reinterpret_cast<LPVOID>(const_cast<char *>(envBlock.c_str())),
					Rain::getPathDir(requestFilePath).c_str(),
					&sinfo,
					&pinfo)) {  //try to fail peacefully
					Rain::errorAndCout(GetLastError(), "Error while starting cgi script " + requestFilePath);
					return -1;
				}

				//pipe the requestBody to the in pipe of the script

				//if we are processing a POST request, pipe the request body into the in pipe and redirect it to the script
				for (std::size_t a = 0; a < bodyBlock.length();) {
					static DWORD dwWritten;
					static BOOL bSuccess;
					if (a + cdParam.fileBufLen >= bodyBlock.length())  //if the current buffer will pipe everything in, make sure not to exceed the end
						bSuccess = WriteFile(g_hChildStd_IN_Wr, bodyBlock.c_str() + a, static_cast<DWORD>(bodyBlock.length() - a), &dwWritten, NULL);
					else  //there's still a lot to pipe in, so fill the buffer and pipe it in
						bSuccess = WriteFile(g_hChildStd_IN_Wr, bodyBlock.c_str() + a, static_cast<DWORD>(cdParam.fileBufLen), &dwWritten, NULL);

					if (!bSuccess) {  //something went wrong while piping input, try to fail peacefully
						Rain::reportError(GetLastError(), "error while piping request body to cgi script; request body:\n" + bodyBlock);
						return -1;
					}
					a += dwWritten;
				}
				CloseHandle(g_hChildStd_IN_Wr);

				//wait for cgi script to finish, up to a timeout
				WaitForInputIdle(pinfo.hProcess, config["http-cgi-to"].i());

				CloseHandle(pinfo.hThread);
				CloseHandle(g_hChildStd_OUT_Wr);
				CloseHandle(g_hChildStd_IN_Rd);

				//compose response
				//get buffered output from the script, and send it over the socket buffered
				for (;;) {
					static DWORD dwRead;
					if (!ReadFile(g_hChildStd_OUT_Rd, cdParam.buffer, static_cast<DWORD>(cdParam.fileBufLen), &dwRead, NULL)) {  //maybe error
						DWORD error = GetLastError();
						if (error == ERROR_BROKEN_PIPE) {
							//the pipe broke because the process has shut it down, this is okay
							break;
						} else if (error == ERROR_OPERATION_ABORTED) {
							//thread probably shut down because client disconnect; terminate the process and exit
							TerminateProcess(pinfo.hProcess, 0);
							break;
						} else {  //actually bad, terminate process
							TerminateProcess(pinfo.hProcess, 0);
							Rain::errorAndCout(error, "Something went wrong with ReadFile.");
							break;
						}
					}

					//check if we want to terminate here if the thread is already terminated
					if (cdParam.disconnectStarted) {
						TerminateProcess(pinfo.hProcess, 0);
						break;
					}

					if (dwRead == 0) {
						//nothing left in pipe, so script must have finished
						break;
					}

					//send buffer through socket
					responseBody = std::string(cdParam.buffer, dwRead);
					if (!Rain::sendRawMessage(cSocket, &responseBody)) {
						Rain::errorAndCout(GetLastError(), "Error while sending response to client; responseBody part: " + responseBody);
						return -1;
					}
				}
				CloseHandle(g_hChildStd_OUT_Rd);
				CloseHandle(pinfo.hProcess);

				//return 1 to terminate socket here
				return 1;
			} else {
				//not a cgi script, must be a file request
				//extract file extension
				std::string requestFileExt = requestFilePath.substr(requestFilePath.rfind(".") + 1, std::string::npos);
				Rain::strToLower(&requestFileExt);

				std::string contentType;
				if (config["http-content"].has(requestFileExt)) {
					contentType = config["http-content"][requestFileExt].s();
				} else {
					contentType = "application/octet-stream";
				}

				//compose response
				responseStatus = "HTTP/1.1 200 OK";
				responseHeaders["content-type"] = contentType + ";charset=UTF-8";

				//get file length and set headers
				std::size_t fileSize = Rain::getFileSize(requestFilePath),
					fileBegin = 0,
					fileEnd = fileSize - 1;

				//make sure to parse range requests (only single range for now)
				if (headers.find("range") != headers.end()) {
					std::string range = headers["range"].substr(headers["range"].find("=") + 1);
					std::size_t rangeDelim = range.find("-");
					fileBegin = Rain::strToT<std::size_t>(range.substr(0, rangeDelim));
					if (rangeDelim != range.length() - 1) {
						fileEnd = Rain::strToT<std::size_t>(range.substr(rangeDelim + 1));
					}
				}

				responseHeaders["content-length"] = Rain::tToStr(fileEnd - fileBegin + 1);
				if (responseHeaders.find("content-range") != responseHeaders.end()) {
					responseHeaders["content-range"] = "bytes " + Rain::tToStr(fileBegin) + "-" + Rain::tToStr(fileEnd) + "/" + Rain::tToStr(fileSize);
				}
				if (responseHeaders.find("server-version") != responseHeaders.end()) {
					responseHeaders["server-version"] = getVersionStr();
				}

				//send what we know
				std::string response = responseStatus + Rain::CRLF;
				for (auto it : responseHeaders)
					response += it.first + ":" + it.second + Rain::CRLF;
				response += Rain::CRLF;
				if (!Rain::sendRawMessage(cSocket, &response)) {
					Rain::errorAndCout(GetLastError(), "error while sending response to client; response: " + response);
					return -1;
				}

				//send file buffered
				std::ifstream fileIn(requestFilePath, std::ios_base::binary);
				fileIn.seekg(fileIn.beg + fileBegin);
				std::size_t fileLen = Rain::strToT<std::size_t>(responseHeaders["content-length"]),
					actualRead;
				for (std::size_t a = fileBegin; a <= fileEnd; a += cdParam.fileBufLen) {
					actualRead = min(cdParam.fileBufLen, fileEnd + 1 - a);
					fileIn.read(cdParam.buffer, actualRead);
					if (!Rain::sendRawMessage(cSocket, cdParam.buffer, static_cast<int>(actualRead))) {
						Rain::errorAndCout(GetLastError(), "error while sending response to client; response segment: " + std::string(cdParam.buffer, actualRead));
						return -1;
					}
				}
				fileIn.close();
			}

			//depending on the "connection" header, close or keep open the socket
			if (Rain::strToLower(headers["connection"]) == "keep-alive")
				return 0;
			else
				return 1;
		}

		void parseHeaders(std::stringstream &headerStream, std::map<std::string, std::string> &headers) {
			std::string key = "", value;
			std::getline(headerStream, key, ':');

			while (key.length() != 0) {
				std::getline(headerStream, value);
				Rain::strTrimWhite(&value);
				Rain::strTrimWhite(&key);
				headers[Rain::strToLower(key)] = value;
				key = "";
				std::getline(headerStream, key, ':');
			}
		}
	}  // namespace HTTPServer
}  // namespace Emilia