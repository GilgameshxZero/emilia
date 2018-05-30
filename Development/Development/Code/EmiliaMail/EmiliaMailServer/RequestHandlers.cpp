#include "RequestHandlers.h"

namespace Monochrome3 {
	namespace EmiliaMailServer {
		int HRRecvEhlo(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			//todo: confirm ehlo
			response << "250-Emilia is best girl!\r\n"
				"250 AUTH LOGIN PLAIN CRAM-MD5" << "\r\n";
			rtParam.smtpWaitFunc = waitData;
			return 0;
		}
		int HRRecvData(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			//wait for the DATA message to come in
			std::string messCpy = rtParam.accMess;
			Rain::strTrim(messCpy);
			if (messCpy == "DATA") {
				rtParam.smtpWaitFunc = waitSendMail;
				response << config["data354"] << "\r\n";
			} else if (messCpy.find("AUTH PLAIN") != std::string::npos)
				return waitAuthLogin(rtParam, config, response);
			else { //otherwise interpret headers
				std::size_t colonLoc = rtParam.accMess.find(":");
				if (colonLoc == std::string::npos) //no colon, not valid header
					return 1;
				std::string key = rtParam.accMess.substr(0, colonLoc),
					value = rtParam.accMess.substr(colonLoc + 1, std::string::npos);
				Rain::strTrim(key);
				Rain::strTrim(value);
				value = value.substr(1, value.length() - 2); //trim the <>
				rtParam.smtpHeaders[key] = value;
				response << config["headers250"] << "\r\n";
			}
			return 0;
		}
		int HRRecvSendMail(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			if (rtParam.accMess.length() < 5 || rtParam.accMess.substr(rtParam.accMess.length() - 5, 5) != "\r\n.\r\n") {//message not complete, save it somewhere and receive more
				rtParam.emailBody += rtParam.accMess;
				return 0;
			}
			response << config["data250"] << "\r\n";
			rtParam.emailBody += rtParam.accMess;
			rtParam.smtpWaitFunc = waitQuit;
			rtParam.smtpSuccess = true;
			return 0;
		}
		int HRRecvQuit(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			std::string messCpy = rtParam.accMess;
			Rain::strTrim(messCpy);
			if (messCpy == "QUIT") {
				response << config["waitQuit221"] << "\r\n";
				return 1;
			} else {
				response << config["waitQuit502"] << "\r\n";
				return 0;
			}

			//send a request to port 25 on localhost to create a thread to send the mail
		}
		int HRRecvAuthLogin(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			response << "235 Emilia loves the world!\r\n";
			rtParam.smtpWaitFunc = waitData;
			return 0;
		}

		int HRSendRequest(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			std::map<std::string, std::string> &config = *rtParam.pLTParam->config;

			//start up client to forward mail to forwarding address, if smtp was sucessful
			if (rtParam.smtpSuccess) {
				rtParam.log += Rain::getTime() + " SMTP success with " + Rain::getClientNumIP(rtParam.pLTParam->cSocket) + "\r\n";

				//use the smtp client to send the email to the forward address
				//make sure the forward options are known
				//read these options every time we come here
				std::map<std::string, std::string> forwardConfig;
				Rain::readParameterFile(config["forwardConfig"], forwardConfig);


				//create CSM to send the mail

			} else {
				rtParam.log += Rain::getTime() + " SMTP was not successful with " + Rain::getClientNumIP(rtParam.pLTParam->cSocket) + "\r\n";
			}

			//output the entire log of the recvThread to the console and file now
			Rain::rainCoutF(rtParam.log);
			Rain::fastOutputFileRef(config["logFile"], rtParam.log, true);

			//use postmessage here because we want the thread of the window to process the message, allowing destroywindow to be called
			//WM_RAINAVAILABLE + 1 is the end message
			PostMessage(rtParam.pLTParam->rainWnd.hwnd, WM_LISTENWNDEND, 0, 0);

			//free WSA2RecvFuncParam here, since recvThread won't need it anymore
			delete &rtParam;
			return 0;
		}
	}
}