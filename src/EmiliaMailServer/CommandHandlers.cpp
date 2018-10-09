#include "CommandHandlers.h"

namespace Monochrome3 {
	namespace EmiliaMailServer {
		int CHExit(CommandHandlerParam &cmhParam) {
			return 1;
		}
		int CHHelp(CommandHandlerParam &cmhParam) {
			Rain::tsCout("Available commands: exit, help\r\n");
			return 0;
		}
	}
}