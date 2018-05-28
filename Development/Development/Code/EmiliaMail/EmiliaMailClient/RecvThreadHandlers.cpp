#include "RecvThreadHandlers.h"

namespace Monochrome3 {
	namespace EMTSMTPClient {
		int onProcessMessage(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			std::map<std::string, std::string> &config = *rtParam.config;

			//preliminarily test that message is done receiving
			rtParam.accMess += rtParam.message;
			if (rtParam.accMess.substr(rtParam.accMess.length() - 2, 2) != "\r\n") //todo
				return 0;

			//process the accumulated message
			std::stringstream response;
			int ret = rtParam.smtpWaitFunc(rtParam, config, response);
			Rain::sendText(*rtParam.sSocket, response.str());

			std::cout << rtParam.accMess << response.str();
			Rain::fastOutputFile(config["logFile"], rtParam.accMess + response.str(), true);
			rtParam.accMess = "";

			return ret;
		}
		void onRecvInit(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			rtParam.mainMutex.lock();
			rtParam.smtpWaitFunc = waitEhlo;
		}
		void onRecvExit(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			rtParam.mainMutex.unlock();
			rtParam.socketActive = false;
		}

		int waitEhlo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			if (Rain::getSMTPStatusCode(rtParam.message) != 220) //confirm status code
				return 1;

			response << "EHLO " << config["ehloResponse"] << "\r\n";
			rtParam.smtpWaitFunc = waitMailFrom;
			return 0;
		}
		int waitMailFrom(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			if (Rain::getSMTPStatusCode(rtParam.message) != 250) //confirm status code
				return 1;

			response << "MAIL FROM:<" << config["mailFrom"] << ">\r\n";
			rtParam.smtpWaitFunc = waitRcptTo;
			return 0;
		}
		int waitRcptTo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			if (Rain::getSMTPStatusCode(rtParam.message) != 250) //confirm status code
				return 1;

			response << "RCPT TO:<" << config["rcptTo"] << ">\r\n";
			rtParam.smtpWaitFunc = waitData;
			return 0;
		}
		int waitData(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			if (Rain::getSMTPStatusCode(rtParam.message) != 250) //confirm status code
				return 1;

			response << "DATA\r\n";
			rtParam.smtpWaitFunc = waitSendMail;
			return 0;
		}
		int waitSendMail(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			if (Rain::getSMTPStatusCode(rtParam.message) != 354) //confirm status code
				return 1;

			//if rawBody is yes, then send the body without setting any of the headers
			if (config["rawBody"] == "yes")
				response << config["emailBodyData"];
			else
				response << "MIME-Version: 1.0\r\n"
					<< "Message-ID: <" << Rain::getTime() << rand() << "@smtp.emilia-tan.com>\r\n" //todo
					<< "Date: " << Rain::getTime("%a, %e %b %G %T %z") << "\r\n"
					<< "From: " << config["fromName"] << " <" << (config["fromEmail"] == "mailFrom" ? config["mailFrom"] : config["fromEmail"]) << ">\r\n"
					<< "Subject: " << config["emailSubject"] << "\r\n"
					<< "To: " << (config["toEmail"] == "rcptTo" ? config["rcptTo"] : config["toEmail"]) << "\r\n"
					<< "Content-Type: text/plain; charset=\"UTF-8\"\r\n"
					<< "\r\n"
					<< config["emailBodyData"] << "\r\n"
					<< ".\r\n";
			rtParam.smtpWaitFunc = waitQuit;
			return 0;
		}
		int waitQuit(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			if (Rain::getSMTPStatusCode(rtParam.message) != 250) //confirm status code
				return 1;

			response << "QUIT\r\n";
			rtParam.smtpWaitFunc = waitEnd;
			return 0;
		}
		int waitEnd(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			if (Rain::getSMTPStatusCode(rtParam.message) != 221) //confirm status code
				*rtParam.clientSuccess = false;
			else
				*rtParam.clientSuccess = true;
			return 1;
		}
	}
}