#include "CommandHandlers.h"

namespace Monochrome3 {
	namespace EmiliaSiteServer {
		int CHExit(CommandHandlerParam &cmhParam) {
			return 1;
		}
		int CHHelp(CommandHandlerParam &cmhParam) {
			Rain::tsCout("Available commands: exit, help\r\n");
			return 0;
		}
	}
}