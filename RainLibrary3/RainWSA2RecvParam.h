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
		WSA2RecvParam(SOCKET *sock, std::string *message, int buflen, void *funcparam, WSA2RecvPMFunc OnProcessMessage, WSA2RecvInitFunc OnRecvInit, WSA2RecvExitFunc OnRecvEnd);

		SOCKET *sock;
		std::string *message;
		int buflen;
		void *funcparam;
		WSA2RecvPMFunc OnProcessMessage; //return nonzero to terminate recv
		WSA2RecvInitFunc OnRecvInit;
		WSA2RecvExitFunc OnRecvEnd;
	};
}