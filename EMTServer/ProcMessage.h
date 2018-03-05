#pragma once
#include "../RainLibrary3/RainLibraries.h"
#include "ListenThreadNode.h"
#include "ListenThreadParam.h"
#include "ListenWndParam.h"
#include "RecvFuncParam.h"

namespace Mono3 {
	int ProcClientMess(void *param);

	//called when socket communication is finished from client end
	int ProcFullMess(RecvFuncParam *rfparam);
}