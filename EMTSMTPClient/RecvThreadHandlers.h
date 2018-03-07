#pragma once

#include "RecvThreadParam.h"

namespace Mono3 {
	namespace SMTPClient {
		int onProcessMessage(void *funcParam);
		void onRecvInit(void *funcParam);
		void onRecvEnd(void *funcParam);

		int waitEhlo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response);
		int waitMailFrom(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response);
		int waitRcptTo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response);
		int waitData(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response);
		int waitSendMail(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response);
		int waitQuit(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response);
		int waitEnd(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response);
	}
}