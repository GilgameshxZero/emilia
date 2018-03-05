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

		//socket between server and client
		SOCKET *socket;

		//place where message is stored for use by any RecvFuncs
		std::string *message;

		//length of buffer for recv
		std::size_t bufLen;

		//parameter to be passed to RecvFuncs
		void *funcParam;

		//called whenever recv returns something to the buffer
		//return nonzero to terminate recv loop
		WSA2RecvPMFunc onProcessMessage;

		//called when RecvThread is about to start or end
		WSA2RecvInitFunc onRecvInit;
		WSA2RecvExitFunc onRecvEnd;
	};
}