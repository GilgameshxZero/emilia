#pragma once
#include "rain-aeternum/rain-libraries.h"

namespace Emilia {
	const enum class CMD_OPTION_R { NONE, CRASH, DEPLOY };
	extern const int MAX_PROJECT_DIR_SEARCH;
	extern const std::string RESTART_SHELL_SCRIPT,
		DEFAULT_CONFIGURATION,
		PROJECT_DIR,
		PROJECT_INDEX,
		PROJECT_CONFIG;
}