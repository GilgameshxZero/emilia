#pragma once

#include <string>
#include <Windows.h>

namespace Rain {
	class RecvHandlerParam {
	public:
		//event handler function type
		typedef int(*EventHandler) (void *);

		RecvHandlerParam();
		RecvHandlerParam(SOCKET *socket, std::string *message, std::size_t bufLen, void *funcParam, EventHandler onConnect, EventHandler onMessage, EventHandler onDisconnect);

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
		EventHandler onMessage;

		//called when RecvThread is about to start or end
		//return nonzero to terminate recv immediately
		EventHandler onConnect;
		EventHandler onDisconnect;
	};
}