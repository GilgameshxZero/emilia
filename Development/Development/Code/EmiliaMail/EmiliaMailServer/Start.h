#pragma once

#include "../../Common/RainAeternum/RainLibraries.h"

#include "RecvThreadHandlers.h"
#include "RecvThreadParam.h"
#include "ListenThread.h"

#include <WinDNS.h>

#pragma comment (lib, "Dnsapi.lib")

namespace Monochrome3 {
	namespace EmiliaMailServer {
		int start(int argc, char* argv[]);
	}
}