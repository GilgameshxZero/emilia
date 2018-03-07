#include "RecvThreadHandlers.h"

namespace Mono3 {
	namespace SMTPClient {
		int onProcessMessage(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			std::map<std::string, std::string> &config = *rtParam.config;

			//send response based on state
			std::stringstream response;
			if (rtParam.state == "HELO") {
				rtParam.state = "FROM";
				response << "EHLO " << config["ehloResponse"] << "\r\n";
			} else if (rtParam.state == "FROM") {
				rtParam.state = "TO";
				response << "MAIL FROM:<" << config["fromEmail"] << ">\r\n";
			} else if (rtParam.state == "TO") {
				rtParam.state = "DATA";
				response << "RCPT TO:<" << config["toEmail"] << ">\r\n";
			} else if (rtParam.state == "DATA") {
				rtParam.state = "actualData";
				response << "DATA\r\n";
			} else if (rtParam.state == "actualData") {
				rtParam.state = "QUIT";
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
			} else if (rtParam.state == "QUIT") {
				rtParam.state = "finished";
				response << "QUIT\r\n";
			} else if (rtParam.state == "finished")
				return 1;
			Rain::sendText(*rtParam.sSocket, response.str());

			//logging
			std::cout << rtParam.message << response.str();
			std::string logData = rtParam.message + response.str();
			Rain::fastOutputFile(config["logFile"], logData, true);

			return 0;
		}
		void onRecvInit(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			rtParam.state = "HELO";
			rtParam.mainMutex.lock();
		}
		void onRecvEnd(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			rtParam.mainMutex.unlock();
		}
	}
}