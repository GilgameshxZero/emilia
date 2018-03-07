#include "RecvThreadHandlers.h"

namespace Mono3 {
	namespace SMTPClient {
		int onProcessMessage(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			std::map<std::string, std::string> &config = *rtParam.config;

			//preliminarily test that message is done receiving
			rtParam.accMess += rtParam.message;
			if (rtParam.accMess.substr(rtParam.accMess.length() - 2, 2) != "\r\n")
				return 0;

			//process the accumulated message
			std::stringstream response;
			rtParam.smtpWaitFunc(rtParam, config, response);
			int ret = Rain::sendText(*rtParam.sSocket, response.str());

			std::cout << rtParam.accMess << response.str();
			Rain::fastOutputFile(config["logFile"], rtParam.accMess + response.str(), true);
			rtParam.accMess = "";

			return 0;
		}
		void onRecvInit(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			rtParam.mainMutex.lock();
			rtParam.smtpWaitFunc = waitEhlo;
		}
		void onRecvEnd(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			rtParam.mainMutex.unlock();
			rtParam.socketActive = false;
		}

		int waitEhlo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			response << "EHLO " << config["ehloResponse"] << "\r\n";
			rtParam.smtpWaitFunc = waitMailFrom;
			return 0;
		}
		int waitMailFrom(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			response << "MAIL FROM:<" << config["fromEmail"] << ">\r\n";
			rtParam.smtpWaitFunc = waitRcptTo;
			return 0;
		}
		int waitRcptTo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			response << "RCPT TO:<" << config["toEmail"] << ">\r\n";
			rtParam.smtpWaitFunc = waitData;
			return 0;
		}
		int waitData(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			response << "DATA\r\n";
			rtParam.smtpWaitFunc = waitSendMail;
			return 0;
		}
		int waitSendMail(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			std::string emailBody;
			response << "MIME-Version: 1.0\r\n"
				<< "Message-ID: <" << Rain::getTime() << rand() << "@smtp.emilia-tan.com>\r\n" //todo
				<< "Date: " << Rain::getTime("%a, %e %b %G %T %z") << "\r\n"
				<< "From: " << config["fromName"] << " <" << config["fromEmail"] << ">\r\n"
				<< "Subject: " << config["emailSubject"] << "\r\n"
				<< "To: " << config["toEmail"] << "\r\n"
				<< "Content-Type: text/plain; charset=\"UTF-8\"\r\n"
				<< "\r\n"
				<< Rain::readFullFile(config["emailBody"], emailBody) << "\r\n"
				<< ".\r\n";
			rtParam.smtpWaitFunc = waitQuit;
			return 0;
		}
		int waitQuit(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			response << "QUIT\r\n";
			rtParam.smtpWaitFunc = waitEnd;
			return 0;
		}
		int waitEnd(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			return 1;
		}
	}
}