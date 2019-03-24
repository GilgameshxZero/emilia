#include "network-recv-param.h"

namespace Rain {
	RecvHandlerParam::RecvHandlerParam() {
	}

	RecvHandlerParam::RecvHandlerParam(SOCKET *socket, std::string *message, std::size_t buflen, void *funcParam, EventHandler onConnect, EventHandler onMessage, EventHandler onDisconnect) {
		this->socket = socket;
		this->message = message;
		this->bufLen = bufLen;
		this->funcParam = funcParam;
		this->onConnect = onConnect;
		this->onMessage = onMessage;
		this->onDisconnect = onDisconnect;
	}
}