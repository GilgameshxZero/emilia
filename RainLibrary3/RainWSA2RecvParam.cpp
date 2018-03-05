#include "RainWSA2RecvParam.h"

namespace Rain {
	WSA2RecvParam::WSA2RecvParam() {
	}

	WSA2RecvParam::WSA2RecvParam(SOCKET *socket, std::string *message, int buflen, void *funcParam, WSA2RecvPMFunc onProcessMessage, WSA2RecvInitFunc onRecvInit, WSA2RecvExitFunc onRecvEnd) {
		this->socket = socket;
		this->message = message;
		this->bufLen = bufLen;
		this->funcParam = funcParam;
		this->onProcessMessage = onProcessMessage;
		this->onRecvInit = onRecvInit;
		this->onRecvEnd = onRecvEnd;
	}
}