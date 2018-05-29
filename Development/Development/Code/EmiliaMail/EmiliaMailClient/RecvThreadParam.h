#pragma once

#include "../../Common/RainLibrary3/RainLibraries.h"

namespace Monochrome3 {
	namespace EMTSMTPClient {
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

			//main can reference this to determine if the sending was successful
			bool *clientSuccess;

			//pointer to procMessage func, which we change based on what stage of smtp we are on
			pSMTPWaitFunc smtpWaitFunc;
		};
	}
}