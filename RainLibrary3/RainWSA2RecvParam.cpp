#include "RainWSA2RecvParam.h"

namespace Rain {
	WSA2RecvParam::WSA2RecvParam() {
	}

	WSA2RecvParam::WSA2RecvParam(SOCKET *sock, std::string *message, int buflen, void *funcparam, WSA2RecvPMFunc OnProcessMessage, WSA2RecvInitFunc OnRecvInit, WSA2RecvExitFunc OnRecvEnd) {
		this->sock = sock;
		this->message = message;
		this->buflen = buflen;
		this->funcparam = funcparam;
		this->OnProcessMessage = OnProcessMessage;
		this->OnRecvInit = OnRecvInit;
		this->OnRecvEnd = OnRecvEnd;
	}
}