#pragma once

#include "../RainLibrary3/RainLibraries.h"

namespace Mono3 {
	namespace SMTPClient {
		struct RecvThreadParam {
			//configuration
			std::map<std::string, std::string> *config;

			//connection to server
			SOCKET *sSocket;

			//message from recvThread
			std::string message;

			//the state of the smtp protocol
			std::string state;

			//mutex to lock the main thread while smtp is ongoing
			std::mutex mainMutex;
		};
	}
}