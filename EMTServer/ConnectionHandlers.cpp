#include "ConnectionHandlers.h"

namespace Monochrome3 {
	namespace EMTServer {
		static const std::string headerDelim = "\r\n\r\n";

		void onConnectionInit(void *funcParam) {
			Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam = *reinterpret_cast<Rain::WSA2ListenThreadRecvFuncDelegateParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);

			//create the delegate parameter for the first time
			ConnectionDelegateParam *cdParam = new ConnectionDelegateParam();
			ltrfdParam.delegateParam = reinterpret_cast<void *>(cdParam);

			//delay all possible initialization of rtParam to here
			cdParam->request = "";
			cdParam->requestMethod = "";
			cdParam->contentLength = -1;
			cdParam->headerBlockLength = -1;
		}
		void onConnectionExit(void *funcParam) {
			//free the delegate parameter
			delete reinterpret_cast<Rain::WSA2ListenThreadRecvFuncDelegateParam *>(funcParam)->delegateParam;
		}
		int onConnectionProcessMessage(void *funcParam) {
			Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam = *reinterpret_cast<Rain::WSA2ListenThreadRecvFuncDelegateParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ltrfdParam.delegateParam);

			//accumulate messages until a complete request is found
			//for now, only accept GET requests and non-chunked POST requests (those with a content-length header)
			//of course, for POST requests, we will need to accumulate not just the header but also the body of the request
			//onProcessMessage is also responsible for parsing the header before passing it on to processRequest
			cdParam.request += ltrfdParam.message;

			//if we don't know the requestMethod yet, attempt to calculate it now
			if (cdParam.requestMethod == "") {
				std::size_t firstSpace = cdParam.request.find(' ');

				if (firstSpace != std::string::npos) //if there's a space, then everything before that is the requestMethod
					cdParam.requestMethod = cdParam.request.substr(0, firstSpace);
			}

			//determine whether the request is complete based on the requestMethod and the currently received request
			if (cdParam.requestMethod == "") { //still don't know the request method yet, so we should wait for more messages to come in 
				return 0;
			} else if (cdParam.requestMethod == "GET") { //GET requests complete when the last four characters are "\r\n\r\n"
				if (cdParam.request.substr(cdParam.request.length() - 4, 4) == headerDelim) { //get ready to process the request
					cdParam.contentLength = 0;
					cdParam.headerBlockLength = cdParam.request.length() - headerDelim.length();
				} else //if the last line isn't blank, keep on waiting for more messages
					return 0;
			} else if (cdParam.requestMethod == "POST") { //identify the header block, find the 'content-length' header, and only process the request when the body is of the correct length
				cdParam.headerBlockLength = Rain::rabinKarpMatch(cdParam.request, headerDelim);
				if (cdParam.headerBlockLength == -1) //still waiting on headers
					return 0;

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
					cdParam.contentLength = Rain::strToT<unsigned long long>(headers["content-length"]);
				}

				//accumulate request until body is contentLength long
				unsigned long long requestLength = cdParam.headerBlockLength + 1 + headerDelim.length() + cdParam.contentLength;
				if (cdParam.request.length() < requestLength)
					return 0;
				else if (cdParam.request.length() > requestLength)//for some reason, the request is longer than anticipated, so terminate the socket
					return -1;
			}

			//at this point, we have a complete request and we know the size of the header block
			//so, parse the header and log the request, then send it over to processRequest
			std::map<std::string, std::string> headers;
			std::string headerBlock = cdParam.request.substr(0, cdParam.headerBlockLength),
				bodyBlock = cdParam.request.substr(cdParam.headerBlockLength + headerDelim.length(), cdParam.contentLength); //the body is everything after the header deliminater
			std::stringstream ss;
			std::string requestURI, httpVersion;

			ss << headerBlock;
			ss >> cdParam.requestMethod >> requestURI >> httpVersion;
			parseHeaders(ss, headers);

			//log data
			std::string clientIP = Rain::getClientNumIP(*ltrfdParam.cSocket),
				consoleLog = (clientIP + ": " + cdParam.requestMethod + " " + requestURI).substr(0, 80) + "\n"; //cut off the console log if its too long so that it doesn't stall the other threads
			Rain::rainCoutF(consoleLog);

			Rain::fastOutputFile(ccParam.config->at("serverLog"),
								 Rain::getTime() + "\t" + clientIP + "\t" + Rain::tToStr(cdParam.request.length()) + " bytes\r\n" +
								 cdParam.request + "\r\n" +
								 "--------------------------------------------------------------------------------\r\n", true);

			//send the parsed headers and bodyBlock and other parameters over to processRequest
			int prRet = processRequest(*ltrfdParam.cSocket,
									   *ccParam.config,
									   cdParam.requestMethod,
									   requestURI,
									   httpVersion,
									   headers,
									   bodyBlock);
			//if it decides to keep the connection open after this full request, then reset request-specific parameters for the recvThread
			if (prRet == 0) {
				cdParam.request = "";
				cdParam.requestMethod = "";
				cdParam.contentLength = -1;
				cdParam.headerBlockLength = -1;
			}

			//< 0 is error, 0 is keep-alive, and > 0 is peacefully close
			return prRet;
		}

		int processRequest(SOCKET &cSocket,
						   std::map<std::string, std::string> &config,
						   std::string &requestMethod,
						   std::string &requestURI,
						   std::string &httpVersion,
						   std::map<std::string, std::string> &headers,
						   std::string &bodyBlock) {
			//a complete, partially parsed, request has come in; see onProcessMessage for the methods that we process and those that we don't
			//we can return 0 to keep socket open, and nonzero to close the socket

			//only accept HTTP version 1.1
			if (httpVersion != "HTTP/1.1")
				return -1;

			//decompose the requestURI
			std::size_t questionMarkPos = requestURI.find("?"),
				hashtagPos = requestURI.find("#");
			std::string requestFilePath = requestURI.substr(0, questionMarkPos),
				requestQuery = "", requestFragment = "";
			if (questionMarkPos != std::string::npos)
				requestQuery = requestURI.substr(questionMarkPos + 1, hashtagPos);
			if (hashtagPos != std::string::npos)
				requestFragment = requestURI.substr(hashtagPos + 1, std::string::npos);

			//parse the URIs
			if (requestFilePath[0] == '/') //relative filepaths should not begin with a slash
				requestFilePath = requestFilePath.substr(1, std::string::npos);
			requestFilePath = Rain::decodeURL(requestFilePath);
			requestQuery = Rain::decodeURL(requestQuery);
			requestFragment = Rain::decodeURL(requestFragment);
			for (int a = 0; a < requestFilePath.size(); a++)
				if (requestFilePath[a] == '/')
					requestFilePath[a] = '\\';

			//decompose requestFilePath and check if we need to substitute a default file
			std::string requestFile = requestFilePath.substr(requestFilePath.rfind("\\") + 1, std::string::npos),
				requestFileDir = requestFilePath.substr(0, requestFilePath.length() - requestFile.length());
			if (requestFile.length() == 0)
				requestFile = config["dirDefaultFile"];
			std::string requestFilePathRel = requestFileDir + requestFile; //short server path
			requestFileDir = Rain::getWorkingDirectory() + config["serverRootDir"] + requestFileDir;
			requestFilePath = requestFileDir + requestFile; //recompose the combined filepath; it is now absolute path

			//make sure cgi scripts are parsed
			static bool cgiScriptsParsed = false;
			static std::set<std::string> cgiScripts;
			if (!cgiScriptsParsed) { //if this is the first time we're here, we also need to read the cgiScripts config file and get a list of all the cgiScripts
				std::ifstream cgiScriptsConfigIn(config["cgiScripts"], std::ios::binary);
				while (cgiScriptsConfigIn.good()) {
					static std::string line;
					std::getline(cgiScriptsConfigIn, line);
					Rain::strTrim(line);
					if (line.length() > 0)
						cgiScripts.insert(line);
				}
				cgiScriptsConfigIn.close();
				cgiScriptsParsed = true;
			}

			//make sure custom response headers are parsed
			static bool customHeadersParsed = false;
			static std::map<std::string, std::string> customHeaders;
			if (!customHeadersParsed) {
				Rain::readParameterFile(config["customHeaders"], customHeaders);

				//add server versioning info to "server" header
				customHeaders["server"] = customHeaders["server"] + " (ver. " + Rain::tToStr(VERSION_MAJOR) + "." + Rain::tToStr(VERSION_MINOR) + "." + Rain::tToStr(VERSION_REVISION) + "." + Rain::tToStr(VERSION_BUILD) + ")";

				customHeadersParsed = true;
			}

			//make sure 404 response is parsed
			static bool notFound404Parsed = false;
			static std::string notFound404HTML;
			if (!notFound404Parsed) {
				Rain::readFullFile(config["404HTML"], notFound404HTML);
				notFound404Parsed = true;
			}

			//response headers that we actually send back should be built on top of customHeaders, so copy customHeaders
			std::map<std::string, std::string> responseHeaders(customHeaders);
			std::string responseStatus, responseBody;

			//assert that the requestFile exists; if not, send back prespecified 404 file
			if (!Rain::fileExists(requestFilePath)) {
				responseStatus = "HTTP/1.1 404 Not Found";
				responseBody = notFound404HTML;
				responseHeaders["content-length"] = Rain::tToStr(responseBody.length());
				responseHeaders["content-type"] = "text/html";

				std::string response = responseStatus + "\r\n";
				for (auto it : responseHeaders)
					response += it.first + ":" + it.second + "\r\n";
				response += "\r\n" + responseBody;
				if (Rain::sendText(cSocket, response.c_str(), response.length())) {
					Rain::reportError(GetLastError(), "error while sending response to client; response: " + response);
					return -7;
				}
			} else if (cgiScripts.find(requestFilePathRel) != cgiScripts.end()) { //branch if the file is a cgi script
				//run the file at FilePath as a cgi script, and respond with its content
				//set the current environment block as well as additional environment parameters for the script
				std::string envBlock;
				Rain::appendEnvVar(envBlock, "QUERY_STRING=" + requestQuery);
				std::map<std::string, std::string>::iterator iterator = headers.find("referer");
				if (iterator != headers.end())
					Rain::appendEnvVar(envBlock, "HTTP_REFERER=" + iterator->second);
				iterator = headers.find("content-length");
				if (iterator != headers.end())
					Rain::appendEnvVar(envBlock, "CONTENT_LENGTH=" + iterator->second);
				iterator = headers.find("host");
				if (iterator != headers.end())
					Rain::appendEnvVar(envBlock, "HTTP_HOST=" + iterator->second);

				//append current environment block
				LPCH curEnvBlock = GetEnvironmentStrings();
				int prevVarBeg = 0;
				for (int a = 0; ; a++) {
					if (curEnvBlock[a] == '\0') {
						Rain::appendEnvVar(envBlock, std::string(curEnvBlock + prevVarBeg + 1, curEnvBlock + a));
						prevVarBeg = a;
						if (curEnvBlock[a + 1] == '\0')
							break;
					}
				}

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
					Rain::reportError(GetLastError(), "error while setting up pipe for cgi script " + requestFilePath);
					return -2; //something went wrong, terminate socket, try to fail peacefully
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
					DETACHED_PROCESS,
					reinterpret_cast<LPVOID>(const_cast<char *>(envBlock.c_str())),
					Rain::getPathDirectory(requestFilePath).c_str(),
					&sinfo,
					&pinfo)) { //try to fail peacefully
					Rain::reportError(GetLastError(), "error while starting cgi script " + requestFilePath);
					return -3;
				}

				//pipe the requestBody to the in pipe of the script

				//if we are processing a POST request, pipe the request body into the in pipe and redirect it to the script
				static std::size_t cgiInPipeBufLen = Rain::strToT<std::size_t>(config["cgiInPipeBufLen"]);
				for (std::size_t a = 0; a < bodyBlock.length();) {
					static DWORD dwWritten;
					static BOOL bSuccess;
					if (a + cgiInPipeBufLen >= bodyBlock.length()) //if the current buffer will pipe everything in, make sure not to exceed the end
						bSuccess = WriteFile(g_hChildStd_IN_Wr, bodyBlock.c_str() + a, static_cast<DWORD>(bodyBlock.length() - a), &dwWritten, NULL);
					else //there's still a lot to pipe in, so fill the buffer and pipe it in
						bSuccess = WriteFile(g_hChildStd_IN_Wr, bodyBlock.c_str() + a, static_cast<DWORD>(cgiInPipeBufLen), &dwWritten, NULL);

					if (!bSuccess) { //something went wrong while piping input, try to fail peacefully
						Rain::reportError(GetLastError(), "error while piping request body to cgi script; request body:\n" + bodyBlock);
						return -4;
					}
					a += dwWritten;
				}
				CloseHandle(g_hChildStd_IN_Wr);

				//wait for cgi script to finish, up to a timeout
				WaitForInputIdle(pinfo.hProcess, Rain::strToT<DWORD>(config["cgiMaxIdleTime"]));

				CloseHandle(pinfo.hThread);
				CloseHandle(g_hChildStd_OUT_Wr);
				CloseHandle(g_hChildStd_IN_Rd);

				//compose response
				//get buffered output from the script, and send it over the socket buffered
				responseStatus = "HTTP/1.1 200 OK";
				if (Rain::sendText(cSocket, responseStatus)) {
					Rain::reportError(GetLastError(), "error while sending response to client; responseStatus: " + responseStatus);
					return -7;
				}

				static std::size_t cgiOutPipeBufLen = Rain::strToT<std::size_t>(config["cgiOutPipeBufLen"]);
				CHAR *chBuf = new CHAR[cgiOutPipeBufLen];
				for (;;) {
					static DWORD dwRead;
					if (!ReadFile(g_hChildStd_OUT_Rd, chBuf, static_cast<DWORD>(cgiOutPipeBufLen), &dwRead, NULL)) { //maybe error
						DWORD error = GetLastError();
						if (error == ERROR_BROKEN_PIPE) //the pipe broke because the process has shut it down, this is okay
							break;
						else { //actually bad, terminate process
							TerminateProcess(pinfo.hProcess, 0);
							Rain::reportError(error, "something went wrong with ReadFile");
							break;
						}
					}
					if (dwRead == 0) //nothing left in pipe
						break;

					//send buffer through socket
					responseBody = std::string(chBuf, dwRead);
					if (Rain::sendText(cSocket, responseBody)) {
						Rain::reportError(GetLastError(), "error while sending response to client; responseBody part: " + responseBody);
						return -7;
					}
				}
				delete[] chBuf;
				CloseHandle(g_hChildStd_OUT_Rd);
				CloseHandle(pinfo.hProcess);
			} else { //not a cgi script, must be a file request
				//extract file extension
				std::string requestFileExt = requestFile.substr(requestFile.rfind(".") + 1, std::string::npos);
				Rain::toLowercase(requestFileExt);

				//read config to determine contenttype
				static bool contentTypeParsed = false;
				static std::map<std::string, std::string> contentTypeSpec;
				if (!contentTypeParsed) {
					Rain::readParameterFile(config["contentTypeSpec"], contentTypeSpec);
					contentTypeParsed = true;
				}

				std::string contentType;
				std::map<std::string, std::string>::iterator iterator = contentTypeSpec.find(requestFileExt);
				if (iterator == contentTypeSpec.end())
					contentType = config["defaultContentType"];
				else
					contentType = iterator->second;

				//compose response
				responseStatus = "HTTP/1.1 200 OK";
				responseHeaders["content-disposition"] = "inline;filename=" + requestFile;
				responseHeaders["content-type"] = contentType + ";charset=UTF-8";

				//get file length
				std::ifstream fileIn(requestFilePath, std::ios_base::binary);
				std::streampos fileBeg = fileIn.tellg();
				fileIn.seekg(0, fileIn.end);
				responseHeaders["content-length"] = Rain::tToStr(fileIn.tellg() - fileBeg);
				fileIn.seekg(0, fileIn.beg);

				//send what we know
				std::string response = responseStatus + "\r\n";
				for (auto it : responseHeaders)
					response += it.first + ":" + it.second + "\r\n";
				response += "\r\n";
				if (Rain::sendText(cSocket, response)) {
					Rain::reportError(GetLastError(), "error while sending response to client; response: " + response);
					return -7;
				}

				//send file buffered
				static std::size_t fileBufLen = Rain::strToT<std::size_t>(config["fileBufLen"]);
				char *buffer = new char[fileBufLen];
				std::size_t fileLen = Rain::strToT<std::size_t>(responseHeaders["content-length"]);
				for (std::size_t a = 0; a < fileLen; a += fileBufLen) {
					std::size_t actualRead = min(fileBufLen, fileLen - a);
					fileIn.read(buffer, actualRead);
					if (Rain::sendText(cSocket, buffer, actualRead)) {
						Rain::reportError(GetLastError(), "error while sending response to client; response segment: " + std::string(buffer, actualRead));
						return -7;
					}
				}
				delete[] buffer;
				fileIn.close();
			}

			//depending on the "connection" header, close or keep open the socket
			if (Rain::toLowercase(headers["connection"]) == "keep-alive")
				return 0;
			else
				return 1;
		}

		void parseHeaders(std::stringstream &headerStream, std::map<std::string, std::string> &headers) {
			std::string key = "", value;
			std::getline(headerStream, key, ':');

			while (key.length() != 0) {
				std::getline(headerStream, value);
				Rain::strTrim(value);
				Rain::strTrim(key);
				headers[Rain::toLowercase(key)] = value;
				key = "";
				std::getline(headerStream, key, ':');
			}
		}
	}
}