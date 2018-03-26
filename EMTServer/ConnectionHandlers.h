#pragma once

#include "../RainLibrary3/RainLibraries.h"

#include <map>
#include <set>

namespace Monochrome3 {
	namespace EMTServer {
		struct ConnectionCallerParam {
			//global config options
			std::map<std::string, std::string> *config;
		};

		struct ConnectionDelegateParam {
			//accumulated request from messages
			std::string request;

			//GET/POST/etc, request method name; "" if don't know yet
			std::string requestMethod;

			//index of the first character of the end of the block of headers (two newlines), or -1
			std::size_t headerBlockLength;

			//content length header content for POST requests; -1 if don't know yet
			//also identifies the length of the body block
			std::size_t contentLength;
		};

		//handlers for RecvThread
		void onConnectionInit(void *funcParam);
		void onConnectionExit(void *funcParam);
		int onConnectionProcessMessage(void *funcParam);

		//called by RecvThread handlers when a full message comes in
		//header keys are all lowercase
		int processRequest(SOCKET &cSocket,
						   std::map<std::string, std::string> &config,
						   std::string &requestMethod,
						   std::string &requestURI,
						   std::string &httpVersion,
						   std::map<std::string, std::string> &headers,
						   std::string &bodyBlock);

		void parseHeaders(std::stringstream &headerStream, std::map<std::string, std::string> &headers);
	}
}