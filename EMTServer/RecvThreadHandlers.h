#pragma once

#include "RecvThreadParam.h"

#include <set>

namespace Monochrome3 {
	namespace EMTServer {
		//handlers for RecvThread
		void onRecvThreadInit(void *funcParam);
		void onRecvThreadEnd(void *funcParam);
		int onProcessMessage(void *funcParam);

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