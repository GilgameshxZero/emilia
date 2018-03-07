#pragma once

#include "../RainLibrary3/RainLibraries.h"

namespace Mono3 {
	namespace SMTPClient {
		struct RecvThreadParam {
			typedef int(*pSMTPWaitFunc) (RecvThreadParam &, std::map<std::string, std::string> &, std::stringstream &);

			//configuration
			std::map<std::string, std::string> *config;

			//connection to server
			SOCKET *sSocket;

			//message from recvThread, and the accumulated message if one recv wasn't enough to get the whole thing
			std::string message;
			std::string accMess;

			//mutex to lock the main thread while smtp is ongoing
			std::mutex mainMutex;
			bool socketActive;

			//pointer to procMessage func, which we change based on what stage of smtp we are on
			pSMTPWaitFunc smtpWaitFunc;
		};
	}
}