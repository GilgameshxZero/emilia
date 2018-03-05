#pragma once

#include "RecvThreadParam.h"

namespace Mono3 {
	//handlers for RecvThread
	int ProcClientMess(void *param);
	void onRecvThreadEnd(void *funcParam);

	//called by RecvThread handlers when a full message comes in
	int ProcFullMess(RecvThreadParam *rfparam);
}