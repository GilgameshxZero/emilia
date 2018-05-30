#pragma once

#include "../../Common/RainAeternum/RainLibraries.h"

#include "CommandHandlers.h"
#include "ConnectionHandlers.h"

#pragma comment (lib, "Dnsapi.lib")

namespace Monochrome3 {
	namespace EmiliaMailServer {
		int start(int argc, char* argv[]);
	}
}