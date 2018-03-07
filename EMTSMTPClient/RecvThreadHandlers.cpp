#include "RecvThreadHandlers.h"

namespace Mono3 {
	namespace SMTPClient {
		int onProcessMessage(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			std::map<std::string, std::string> &config = *rtParam.config;

			std::stringstream response;
			rtParam.smtpWaitFunc(rtParam, config, rtParam.message, response);
			Rain::sendText(*rtParam.sSocket, response.str());

			std::cout << rtParam.message << response.str();
			Rain::fastOutputFile(config["logFile"], rtParam.message + response.str(), true);

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

		int waitEhlo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response) {
			response << "EHLO " << config["ehloResponse"] << "\r\n";
			rtParam.smtpWaitFunc = waitMailFrom;
			return 0;
		}
		int waitMailFrom(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response) {
			response << "MAIL FROM:<" << config["fromEmail"] << ">\r\n";
			rtParam.smtpWaitFunc = waitRcptTo;
			return 0;
		}
		int waitRcptTo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response) {
			response << "RCPT TO:<" << config["toEmail"] << ">\r\n";
			rtParam.smtpWaitFunc = waitData;
			return 0;
		}
		int waitData(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response) {
			response << "DATA\r\n";
			rtParam.smtpWaitFunc = waitSendMail;
			return 0;
		}
		int waitSendMail(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response) {
			std::string emailBody;
			response << "MIME-Version: 1.0\r\n"
				<< "DKIM-Signature: v=1; a=rsa-sha256; s=dkim; d=emilia-tan.com; h=Received : From : To : Subject : ; l=0; bh=;"
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
		int waitQuit(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response) {
			response << "QUIT\r\n";
			rtParam.smtpWaitFunc = waitEnd;
			return 0;
		}
		int waitEnd(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::string &message, std::stringstream &response) {
			return 1;
		}
	}
}