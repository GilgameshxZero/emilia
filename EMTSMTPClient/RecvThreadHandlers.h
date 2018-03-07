#pragma once

#include "RecvThreadParam.h"

namespace Mono3 {
	namespace SMTPClient {
		int onProcessMessage(void *funcParam);
		void onRecvInit(void *funcParam);
		void onRecvEnd(void *funcParam);
	}
}