#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "update-server-param.h"
#include "update-client-param.h"
#include "update-helper-param.h"

#include <ShellAPI.h>

namespace Emilia {
	namespace UpdateHelper {
		int pullProc(std::string method, CommandHandlerParam &cmhParam, PullProcParam &pullPP, std::string &message, Rain::SocketManager &sm);
		int pushProc(std::string method, CommandHandlerParam &cmhParam, PushProcParam &pushPP, std::string &message, Rain::SocketManager &sm);

		std::string generatePushHeader(std::string root, std::vector<std::string> &files);
	}
}