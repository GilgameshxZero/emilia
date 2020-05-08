#pragma once
#include "../rain/src/rain.hpp"

namespace Emilia {
	namespace HTTPServer {
		struct ConnectionCallerParam {
			std::string project;
			Rain::Configuration *config = NULL;
			Rain::LogStream *logHTTP = NULL;

			//total number of connected clients
			int connectedClients = 0;
		};

		struct ConnectionDelegateParam {
			//accumulated request from messages
			std::string request;

			//GET/POST/etc, request method name; "" if don't know yet
			std::string requestMethod;

			//index of the first character of the end of the block of headers (two newlines), or -1
			std::size_t headerBlockLength = 0;

			//content length header content for POST requests; -1 if don't know yet
			//also identifies the length of the body block
			std::size_t contentLength = 0;

			//allow messages to be processed in a separate thread from recvThread & handlers
			std::thread requestThread;

			//event triggered once each time a new messages is added to the request string
			HANDLE newMessageEvent = NULL;

			//locked when newMessage or request is being changed
			std::mutex requestModifyMutex;

			//set when disconnect function begins
			bool disconnectStarted = false;

			//buffer for sending anything
			std::size_t fileBufLen = 0;
			char *buffer = NULL;
		};
	}
}