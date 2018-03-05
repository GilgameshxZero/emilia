#pragma once

#include "MessageProcParam.h"

namespace Mono3 {
	int ProcClientMess(void *param);

	//onCall functions called by RainWSA2 library
	void OnClientRecvEnd(void *param);

	//called when socket communication is finished from client end
	int ProcFullMess(RecvFuncParam *rfparam);
}