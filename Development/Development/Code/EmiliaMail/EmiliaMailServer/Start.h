#pragma once

#include "../../Common/RainLibrary3/RainLibraries.h"
#include "RecvThreadHandlers.h"
#include "RecvThreadParam.h"
#include "ListenThread.h"

#include <WinDNS.h>

#pragma comment (lib, "Dnsapi.lib")

namespace Monochrome3 {
	namespace EMTSMTPServer {
		int start();
	}
}