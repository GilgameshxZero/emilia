#pragma once

#include "../../Common/RainLibrary3/RainLibraries.h"
#include "RecvThreadHandlers.h"
#include "RecvThreadParam.h"

#include <fcntl.h>
#include <io.h>
#include <WinDNS.h>

#pragma comment (lib, "Dnsapi.lib")

namespace Monochrome3 {
	namespace EMTSMTPClient {
		int start();
	}
}