#pragma once
#include "../../Common/RainLibrary3/RainLibraries.h"

#include <Windows.h>

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		//any socket retreived from here should be authenticated with the server, or returned with an error
		class SocketManager {
			public:
			//returns valid
			int getSocket(SOCKET &socket);

			private:
			SOCKET socket;
		};
	}
}