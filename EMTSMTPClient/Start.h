#pragma once

#include "../RainLibrary3/RainLibraries.h"
#include "RecvThreadHandlers.h"
#include "RecvThreadParam.h"

#include <fcntl.h>
#include <io.h>
#include <WinDNS.h>

#pragma comment (lib, "Dnsapi.lib")

namespace Mono3 {
	namespace SMTPClient {
		int start();
	}
}