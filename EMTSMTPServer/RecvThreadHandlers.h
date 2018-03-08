#pragma once

#include "RecvThreadParam.h"

namespace Mono3 {
	namespace SMTPServer {
		int onProcessMessage(void *funcParam);
		void onRecvThreadInit(void *funcParam);
		void onRecvThreadEnd(void *funcParam);

		int waitEhlo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
		int waitData(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
		int waitSendMail(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
		int waitQuit(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
	}
}