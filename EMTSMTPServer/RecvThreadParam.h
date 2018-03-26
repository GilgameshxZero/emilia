#pragma once

#include "../RainLibrary3/RainLibraries.h"
#include "ListenThreadParam.h"

namespace Monochrome3 {
	namespace EMTSMTPServer {
		struct RecvThreadParam {
			typedef int(*pSMTPWaitFunc) (RecvThreadParam &, std::map<std::string, std::string> &, std::stringstream &);

			//pointer to the listenThread associated with this recvThread
			ListenThreadParam *pLTParam;

			//where recvThread will store messages after buffering for the handlers
			std::string message;

			//accumulated full message
			std::string accMess;

			//smtp stuff received
			std::map<std::string, std::string> smtpHeaders;
			std::string emailBody;

			//pointer to procMessage func, which we change based on what stage of smtp we are on
			pSMTPWaitFunc smtpWaitFunc;

			//whether communications finished successfully
			bool smtpSuccess;

			//accumulated log for the recvThread
			std::string log;
		};
	}
}