#pragma once

#include "ListenThreadParam.h"

#include <queue>

namespace Monochrome3 {
	namespace EMTServer {
		struct RecvThreadParam {
			//pointer to the listenThread associated with this recvThread
			ListenThreadParam *pLTParam;

			//where recvThread will store messages after buffering for the handlers
			std::string message;

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
	}
}