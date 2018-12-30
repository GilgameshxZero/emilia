#pragma once
#pragma comment(lib, "user32.lib")

#include "rain-aeternum/rain-libraries.h"

#include "build-helper.h"

#include <algorithm>
#include <map>
#include <set>

namespace Emilia {
	namespace HTTPServer {
		struct ConnectionCallerParam {
			//global config options
			std::map<std::string, std::string> *config;

			//used to log socket communications from main
			Rain::LogStream *logger;

			//total number of connected clients
			int connectedClients;

			//more config
			std::set<std::string> cgiScripts;
			std::map<std::string, std::string> customHeaders;
			std::string notFound404HTML;
			std::map<std::string, std::string> contentTypeSpec;
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

			//allow messages to be processed in a separate thread from recvThread & handlers
			std::thread requestThread;

			//event triggered once each time a new messages is added to the request string
			HANDLE newMessageEvent;

			//locked when newMessage or request is being changed
			std::mutex requestModifyMutex;

			//set when disconnect function begins
			bool disconnectStarted = false;

			//buffer for sending anything
			std::size_t fileBufLen;
			char *buffer;
		};

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