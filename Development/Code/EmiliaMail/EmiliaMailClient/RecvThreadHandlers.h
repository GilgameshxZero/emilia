#pragma once

#include "RecvThreadParam.h"

namespace Monochrome3 {
	namespace EMTSMTPClient {
		int onProcessMessage(void *funcParam);
		void onRecvInit(void *funcParam);
		void onRecvExit(void *funcParam);

		int waitEhlo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
		int waitMailFrom(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
		int waitRcptTo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
		int waitData(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
		int waitSendMail(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
		int waitQuit(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
		int waitEnd(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response);
	}
}