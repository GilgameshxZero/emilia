#include "RecvThreadHandlers.h"

namespace Monochrome3 {
	namespace EMTSMTPServer {
		int onProcessMessage(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			std::map<std::string, std::string> &config = *rtParam.pLTParam->config;

			//preliminarily test that message is done receiving
			rtParam.accMess += rtParam.message;
			if (rtParam.accMess.substr(rtParam.accMess.length() - 2, 2) != "\r\n") //todo
				return 0;

			//process the accumulated message
			std::stringstream response;
			int ret = rtParam.smtpWaitFunc(rtParam, config, response);
			Rain::sendText(rtParam.pLTParam->cSocket, response.str());

			rtParam.log += rtParam.accMess + response.str();
			rtParam.accMess = "";

			return ret;
		}
		void onRecvThreadInit(void *funcParam) {
			RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
			rtParam.smtpWaitFunc = waitEhlo;
			rtParam.smtpSuccess = false;
		}
		void onRecvThreadEnd(void *funcParam) {
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

				//to pass to smtp client
				std::string clientIn;
				std::stringstream clientInSS;
				clientInSS << "readConfig: no" << "\r\n"
					<< "\r\n"
					<< "smtpPort: 25" << "\r\n"
					<< "recvBufLen: 1024" << "\r\n"
					<< "ehloResponse: emilia-tan.com" << "\r\n"
					<< "maxConnToServ: 3" << "\r\n"
					<< "maxSendAttempt: 3" << "\r\n"
					<< "\r\n"
					<< "logFile: ..\\..\\..\\Auxiliary\\EmiliaMail\\EmiliaMailClient\\log.log" << "\r\n"
					<< "errorLog: ..\\..\\..\\Auxiliary\\EmiliaMail\\EmiliaMailClient\\error.log" << "\r\n"
					<< "memoryLeakLog: ..\\..\\..\\Auxiliary\\EmiliaMail\\EmiliaMailClient\\memory-leaks.log" << "\r\n"
					<< "\r\n"
					<< "rawBody: yes\r\n";

				//depending on the to/from addresses, we either want to forward an email that was intended for @emilia-tan, or send an email from @emilia-tan
				std::size_t atDelimPos = rtParam.smtpHeaders["RCPT TO"].find("@");
				if (atDelimPos == std::string::npos) { //invalid rcpt to header, error ane return
					Rain::reportError(-1, "invalid RCPT TO header " + rtParam.smtpHeaders["RCPT TO"]);
					return;
				}
				if (rtParam.smtpHeaders["RCPT TO"].substr(atDelimPos + 1, std::string::npos) == "emilia-tan.com") { //an email destined for @emilia-tan
					//get address we should forward to
					std::string toEmiliaAddr = rtParam.smtpHeaders["RCPT TO"].substr(0, atDelimPos);
					std::string rcptTo;
					if (forwardConfig.find(toEmiliaAddr) == forwardConfig.end())
						rcptTo = config["defaultForward"];
					else
						rcptTo = forwardConfig[toEmiliaAddr];

					//write input to the client, in the form a parameter file; see client for more details
					clientInSS << "mailFrom: server@emilia-tan.com\r\n"
						<< "rcptTo: " << rcptTo << "\r\n";
				} else { //an email send by another client for us, from @emilia-tan.com
					clientInSS << "mailFrom: " << rtParam.smtpHeaders["MAIL FROM"] << "\r\n"
						<< "rcptTo: " << rtParam.smtpHeaders["RCPT TO"] << "\r\n";
				}
				clientInSS << "\r\n"
					<< "fromEmail: mailFrom\r\n" //doesn't matter
					<< "toEmail: rcptTo\r\n" //doesn't matter
					<< "fromName: _" << "\r\n"
					<< "emailSubject: _" << "\r\n"
					<< "\r\n"
					<< "emailBodyFile: _" << "\r\n"
					<< "emailBodyLen: " << rtParam.emailBody.length() << "\r\n"
					<< "emailBodyData:" << rtParam.emailBody << "\r\n" //no space after colon is important
					<< "\r\n"
					<< "_configEnd_: true" << "\r\n";
				clientIn = clientInSS.str();

				//make sure smtp client only runs one at a time, so that we don't overload ports and cause errors
				//TODO: fix this
				//rtParam.pLTParam->smtpClientMutex->lock();

				//similar to cgi stuff in EMTServer
				std::string clientExePath = Rain::getWorkingDirectory() + config["clientExePath"],
					clientExeDir = Rain::getPathDirectory(clientExePath);

				SECURITY_ATTRIBUTES sa;
				sa.nLength = sizeof(SECURITY_ATTRIBUTES);
				sa.bInheritHandle = TRUE;
				sa.lpSecurityDescriptor = NULL;

				HANDLE g_hChildStd_OUT_Rd = NULL,
					g_hChildStd_OUT_Wr = NULL,
					g_hChildStd_IN_Rd = NULL,
					g_hChildStd_IN_Wr = NULL;

				if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0) ||
					!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) ||
					!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &sa, 0) ||
					!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
					Rain::reportError(GetLastError(), "error while setting up pipes for smtp client " + clientExePath);
					return;
				}

				//execute the script with the pipes
				STARTUPINFO sinfo;
				PROCESS_INFORMATION pinfo;
				ZeroMemory(&sinfo, sizeof(sinfo));
				ZeroMemory(&pinfo, sizeof(pinfo));
				sinfo.cb = sizeof(sinfo);
				sinfo.dwFlags |= STARTF_USESTDHANDLES;
				sinfo.hStdOutput = NULL;
				sinfo.hStdInput = g_hChildStd_IN_Rd;
				if (!CreateProcess(
					clientExePath.c_str(),
					NULL,
					NULL,
					NULL,
					TRUE,
					CREATE_NEW_CONSOLE,
					NULL,
					clientExeDir.c_str(),
					&sinfo,
					&pinfo)) { //try to fail peacefully
					Rain::reportError(GetLastError(), "error while starting smtp client " + clientExePath);
					return;
				}

				//pipe input to the client telling it what to do
				static std::size_t cgiInPipeBufLen = Rain::strToT<std::size_t>(config["smtpClientInBufLen"]);
				for (std::size_t a = 0; a < clientIn.length();) {
					static DWORD dwWritten;
					static BOOL bSuccess;
					if (a + cgiInPipeBufLen >= clientIn.length()) //if the current buffer will pipe everything in, make sure not to exceed the end
						bSuccess = WriteFile(g_hChildStd_IN_Wr, clientIn.c_str() + a, static_cast<DWORD>(clientIn.length() - a), &dwWritten, NULL);
					else //there's still a lot to pipe in, so fill the buffer and pipe it in
						bSuccess = WriteFile(g_hChildStd_IN_Wr, clientIn.c_str() + a, static_cast<DWORD>(cgiInPipeBufLen), &dwWritten, NULL);

					if (!bSuccess) { //something went wrong while piping input, try to fail peacefully
						Rain::reportError(GetLastError(), "error while piping input to smtp client; input:\n" + clientIn);
						return;
					}
					a += dwWritten;
				}
				CloseHandle(g_hChildStd_IN_Wr);

				//wait for client to finish to a timeout
				WaitForInputIdle(pinfo.hProcess, Rain::strToT<DWORD>(config["smtpClientMaxIdle"]));

				CloseHandle(g_hChildStd_OUT_Rd);

				//wait for client to end, up to some time
				WaitForSingleObject(pinfo.hProcess, Rain::strToT<DWORD>(config["smtpClientMaxTimeout"]));

				//if it hasn't ended by then, we have to move on
				TerminateProcess(pinfo.hProcess, 0);
				CloseHandle(pinfo.hProcess);

				CloseHandle(pinfo.hThread);
				CloseHandle(g_hChildStd_OUT_Wr);
				CloseHandle(g_hChildStd_IN_Rd);

				//TODO: fix this
				//rtParam.pLTParam->smtpClientMutex->unlock();
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
		}

		int waitEhlo(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			//todo: confirm ehlo
			response << "250-Emilia is best girl!\r\n"
				"250 AUTH LOGIN PLAIN CRAM-MD5" << "\r\n";
			rtParam.smtpWaitFunc = waitData;
			return 0;
		}
		int waitData(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
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
		int waitSendMail(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
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
		int waitQuit(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			std::string messCpy = rtParam.accMess;
			Rain::strTrim(messCpy);
			if (messCpy == "QUIT") {
				response << config["waitQuit221"] << "\r\n";
				return 1;
			} else {
				response << config["waitQuit502"] << "\r\n";
				return 0;
			}
		}

		int waitAuthLogin(RecvThreadParam &rtParam, std::map<std::string, std::string> &config, std::stringstream &response) {
			response << "235 Emilia loves the world!\r\n";
			rtParam.smtpWaitFunc = waitData;
			return 0;
		}
	}
}