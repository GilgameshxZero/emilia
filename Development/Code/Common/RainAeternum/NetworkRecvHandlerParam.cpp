#include "NetworkRecvHandlerParam.h"

namespace Rain {
	NetworkRecvHandlerParam::NetworkRecvHandlerParam() {
	}

	NetworkRecvHandlerParam::NetworkRecvHandlerParam(SOCKET *socket, std::string *message, std::size_t buflen, void *funcParam, EventHandler onProcessMessage, EventHandler onRecvInit, EventHandler onRecvExit) {
		this->socket = socket;
		this->message = message;
		this->bufLen = bufLen;
		this->funcParam = funcParam;
		this->onProcessMessage = onProcessMessage;
		this->onRecvInit = onRecvInit;
		this->onRecvExit = onRecvExit;
	}
}