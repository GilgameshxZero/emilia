#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "command-handler-param.h"
#include "main.h"
#include "update-client.h"
#include "update-helper.h"

#include <map>
#include <ShellAPI.h>
#include <string>

namespace Emilia {
	typedef int(*CommandMethodHandler)(CommandHandlerParam &);

	//handlers should return nonzero to immediately terminate program
	int CHExit(CommandHandlerParam &cmhParam);
	int CHHelp(CommandHandlerParam &cmhParam);
	int CHConnect(CommandHandlerParam &cmhParam);
	int CHDisconnect(CommandHandlerParam &cmhParam);
	int CHPush(CommandHandlerParam &cmhParam);
	int CHPushExclusive(CommandHandlerParam &cmhParam);
	int CHPull(CommandHandlerParam &cmhParam);
	int CHStart(CommandHandlerParam &cmhParam);
	int CHStop(CommandHandlerParam &cmhParam);
	int CHRestart(CommandHandlerParam &cmhParam);
}