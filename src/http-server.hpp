#pragma once
#include "../rain/src/rain.hpp"

#include "http-server-param.hpp"
#include "utils.hpp"

#pragma comment(lib, "user32.lib")

namespace Emilia {
	namespace HTTPServer {
		//handlers for RecvThread
		int onConnect(void *funcParam);
		int onMessage(void *funcParam);
		int onDisconnect(void *funcParam);

		//thread which processes messages that comes in, parallel with the handlers & recvThread
		int requestThreadFunc(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam);

		//called by RecvThread handlers when a full message comes in
		//header keys are all lowercase
		int processRequest(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam,
			std::string &requestURI,
			std::string &httpVersion,
			std::map<std::string, std::string> &headers,
			std::string &bodyBlock);

		//helper function to parse a header block in a SS
		void parseHeaders(std::stringstream &headerStream, std::map<std::string, std::string> &headers);
	}  // namespace HTTPServer
}  // namespace Emilia