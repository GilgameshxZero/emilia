/*
Standard
*/

/*
WSA2RecvParam and related definitions.
*/

#pragma once

#include "RainWSA2Include.h"

#include <string>

namespace Rain {
	typedef int(*WSA2RecvPMFunc) (void *);
	typedef void(*WSA2RecvInitFunc) (void *);
	typedef void(*WSA2RecvExitFunc) (void *);

	class WSA2RecvParam {
		public:
		WSA2RecvParam();
		WSA2RecvParam(SOCKET *socket, std::string *message, int bufLen, void *funcparam, WSA2RecvPMFunc onProcessMessage, WSA2RecvInitFunc onRecvInit, WSA2RecvExitFunc onRecvEnd);

		SOCKET *socket;
		std::string *message;
		int bufLen;
		void *funcParam;
		WSA2RecvPMFunc onProcessMessage; //return nonzero to terminate recv
		WSA2RecvInitFunc onRecvInit;
		WSA2RecvExitFunc onRecvEnd;
	};
}