#include "RecvThreadHandlers.h"

namespace Mono3 {
	namespace Server {
		static const std::string headerDelim = "\r\n\r\n";

		void onRecvThreadInit(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);

			//delay all possible initialization of rtParam to here
			rtParam.requestMethod = "";
			rtParam.contentLength = -1;
			rtParam.headerBlockLength = -1;
		}
		void onRecvThreadEnd(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);

			//use postmessage here because we want the thread of the window to process the message, allowing destroywindow to be called
			//WM_RAINAVAILABLE + 1 is the end message
			PostMessage(rtParam.pLTParam->rainWnd.hwnd, WM_LISTENWNDEND, 0, 0);

			//free WSA2RecvParam here, since recvThread won't need it anymore
			delete &rtParam;
		}
		int onProcessMessage(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);

			//accumulate messages until a complete request is found
			//for now, only accept GET requests and non-chunked POST requests (those with a content-length header)
			//of course, for POST requests, we will need to accumulate not just the header but also the body of the request
			//onProcessMessage is also responsible for parsing the header before passing it on to processRequest
			rtParam.request += rtParam.message;

			//if we don't know the requestMethod yet, attempt to calculate it now
			if (rtParam.requestMethod == "") {
				std::size_t firstSpace = rtParam.request.find(' ');

				if (firstSpace != std::string::npos) //if there's a space, then everything before that is the requestMethod
					rtParam.requestMethod = rtParam.request.substr(0, firstSpace);
			}

			//determine whether the request is complete based on the requestMethod and the currently received request
			if (rtParam.requestMethod == "") { //still don't know the request method yet, so we should wait for more messages to come in 
				return 0;
			} else if (rtParam.requestMethod == "GET") { //GET requests complete when the last four characters are "\r\n\r\n"
				if (rtParam.request.substr(rtParam.request.length() - 4, 4) == headerDelim) { //get ready to process the request
					rtParam.contentLength = 0;
					rtParam.headerBlockLength = rtParam.request.length() - headerDelim.length();
				} else //if the last line isn't blank, keep on waiting for more messages
					return 0;
			} else if (rtParam.requestMethod == "POST") { //identify the header block, find the 'content-length' header, and only process the request when the body is of the correct length
				rtParam.headerBlockLength = Rain::rabinKarpMatch(rtParam.request, headerDelim);
				if (rtParam.headerBlockLength == -1) //still waiting on headers
					return 0;

				//if don't know contentLength yet from previous times we get here, try again to get it
				if (rtParam.contentLength == -1) {
					//a lot of these parameters are not used, but that's fine, since it's a one time cost
					std::map<std::string, std::string> headers;
					std::string headerBlock = rtParam.request.substr(0, rtParam.headerBlockLength);
					std::stringstream ss;
					std::string requestURI, httpVersion;

					ss << headerBlock;
					ss >> rtParam.requestMethod >> requestURI >> httpVersion;
					parseHeaders(ss, headers);
					rtParam.contentLength = Rain::strToT<unsigned long long>(headers["content-length"]);
				}

				//accumulate request until body is contentLength long
				unsigned long long requestLength = rtParam.headerBlockLength + 1 + headerDelim.length() + rtParam.contentLength;
				if (rtParam.request.length() < requestLength)
					return 0;
				else if (rtParam.request.length() > requestLength)//for some reason, the request is longer than anticipated, so terminate the socket
					return -1;
			}

			//at this point, we have a complete request and we know the size of the header block
			//so, parse the header and log the request, then send it over to processRequest
			std::map<std::string, std::string> headers;
			std::string headerBlock = rtParam.request.substr(0, rtParam.headerBlockLength),
				bodyBlock = rtParam.request.substr(rtParam.headerBlockLength + headerDelim.length(), rtParam.contentLength); //the body is everything after the header deliminater
			std::stringstream ss;
			std::string requestURI, httpVersion;

			ss << headerBlock;
			ss >> rtParam.requestMethod >> requestURI >> httpVersion;
			parseHeaders(ss, headers);

			//log data
			std::string clientIP = Rain::getClientNumIP(rtParam.pLTParam->cSocket),
				consoleLog = (clientIP + ": " + rtParam.requestMethod + " " + requestURI).substr(0, 80) + "\n"; //cut off the console log if its too long so that it doesn't stall the other threads
			Rain::rainCoutF(consoleLog);

			Rain::fastOutputFile(rtParam.pLTParam->config->at("serverLog"),
								 Rain::getTime() + "\t" + clientIP + "\t" + Rain::tToStr(rtParam.request.length()) + " bytes\r\n" +
								 rtParam.request + "\r\n" +
								 "--------------------------------------------------------------------------------\r\n", true);

			//send the parsed headers and bodyBlock and other parameters over to processRequest
			return processRequest(rtParam.pLTParam->cSocket,
								  *rtParam.pLTParam->config,
								  rtParam.requestMethod,
								  requestURI,
								  httpVersion,
								  headers,
								  bodyBlock);
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
				//get the output from the script
				responseStatus = "HTTP/1.1 200 OK";
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
					responseBody += std::string(chBuf, dwRead);
				}
				delete[] chBuf;
				CloseHandle(g_hChildStd_OUT_Rd);
				CloseHandle(pinfo.hProcess);

				//try to decompose the responseBody into headers and body and prepare for responding
				std::size_t headerBlockEnd = Rain::rabinKarpMatch(responseBody, "\r\n\r\n");
				if (headerBlockEnd == -1) {
					Rain::reportError(GetLastError(), "cgi script didn't output with correct format; output:\n" + responseBody);
					return -6;
				}

				std::stringstream ss;
				ss << responseBody.substr(0, headerBlockEnd);
				parseHeaders(ss, responseHeaders);
				responseBody = responseBody.substr(headerBlockEnd + headerDelim.length(), std::string::npos);

				//now, exit the if statement to send the response
			} else { //not a cgi script
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
				Rain::readFullFile(requestFilePath, responseBody);
				responseHeaders["content-disposition"] = "inline; filename=" + requestFile;
				responseHeaders["content-length"] = Rain::tToStr(responseBody.length());
				responseHeaders["content-type"] = contentType + "; charset = UTF-8";
			}

			//now, send back responseStatus, responseHeaders, and responseBody
			std::string response = responseStatus + "\r\n";
			for (auto it : responseHeaders)
				response += it.first + ":" + it.second + "\r\n";
			response += "\r\n" + responseBody;
			if (Rain::sendText(cSocket, response.c_str(), response.length())) {
				Rain::reportError(GetLastError(), "error while sending response to client; response: " + response);
				return -7;
			}

			return 1; //close connection once a full message has been processed
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