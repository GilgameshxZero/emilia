#pragma once

#include "../RainLibrary3/RainLibraries.h"
#include "RecvThreadHandlers.h"
#include "RecvThreadParam.h"
#include "ListenThread.h"

#include <WinDNS.h>

#pragma comment (lib, "Dnsapi.lib")

namespace Mono3 {
	namespace SMTPServer {
		int start();
	}
}