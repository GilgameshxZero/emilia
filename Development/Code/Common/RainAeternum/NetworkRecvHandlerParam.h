/*
Standard
*/

#pragma once

#include "RainWindow.h"
#include "NetworkWSAInclude.h"

#include <string>
#include <unordered_map>

namespace Rain {
	class NetworkRecvHandlerParam {
		public:
		//event handler function type
		typedef int(*EventHandler) (void *);

		NetworkRecvHandlerParam();
		NetworkRecvHandlerParam(SOCKET *socket, std::string *message, std::size_t bufLen, void *funcparam, EventHandler onProcessMessage, EventHandler onRecvInit, EventHandler onRecvExit);

		//socket between server and client
		SOCKET *socket;

		//place where message is stored for use by any EventHandler
		std::string *message;

		//length of buffer for recv
		std::size_t bufLen;

		//parameter to be passed to EventHandler
		void *funcParam;

		//called whenever recv returns something to the buffer
		//return nonzero to terminate recv loop
		EventHandler onProcessMessage;

		//called when RecvThread is about to start or end
		//return nonzero to terminate recv immediately
		EventHandler onRecvInit;
		EventHandler onRecvExit;
	};
}