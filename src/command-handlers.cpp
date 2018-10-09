#include "command-handlers.h"

namespace Emilia {
	int CHExit(CommandHandlerParam &cmhParam) {
		return 1;
	}
	int CHHelp(CommandHandlerParam &cmhParam) {
		Rain::tsCout("Available commands: exit, help\r\n");
		return 0;
	}
}