#include "ConnectionHandlers.h"

namespace Monochrome3 {
	namespace EmiliaMailServer {
		int onConnect(void *param) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(param);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			ssmdhParam.delegateParam = new ConnectionDelegateParam();
			Rain::tsCout("Info: Client ", Rain::getClientNumIP(*ssmdhParam.cSocket), " connected.\r\n");
			fflush(stdout);

			return 0;
		}
		int onMessage(void *param) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(param);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			cdParam.request += *ssmdhParam.message;

			if (cdParam.cType == "") {
				std::size_t firstSpace = cdParam.request.find(' ');
				if (firstSpace != std::string::npos) {
					std::string method = cdParam.request.substr(0, firstSpace);
					if (method == "EHLO") {
						cdParam.cType = "recv";
						cdParam.rcd.reqHandler = HRREhlo;
					} else {
						cdParam.cType = "send";
						cdParam.scd.reqHandler = HRSAuth;
					}
				}
			}

			int ret = 0;
			if (cdParam.cType == "recv") {
				//test if message is done receiving
				if (cdParam.rcd.reqHandler != HRRData) {
					//message terminated by CRLF
					if (cdParam.request.substr(cdParam.request.length() - 2, 2) == "\r\n") {
						//message complete, send to handler
						ret = cdParam.rcd.reqHandler(ssmdhParam);
					}
				} else {
					//message terminated by \r\n.\r\n
					if (cdParam.request.substr(cdParam.request.length() - 5, 5) == "\r\n.\r\n") {
						//message complete, send to handler
						ret = cdParam.rcd.reqHandler(ssmdhParam);
					}
				}
			} else if (cdParam.cType == "send") {
				//message is done if its length is correct; care for multiple message mushed together
				while (true) {
					if (cdParam.scd.requestLength == 0) {
						std::size_t firstSpace = cdParam.request.find(' ');
						if (firstSpace != std::string::npos) {
							cdParam.scd.requestLength = Rain::strToT<std::size_t>(cdParam.request.substr(0, firstSpace));
							cdParam.request = cdParam.request.substr(firstSpace + 1, cdParam.request.length());
						} else
							break;
					}
					if (cdParam.scd.requestLength != 0 && 
						cdParam.scd.requestLength >= cdParam.request.length()) {
						std::string fragment = cdParam.request.substr(cdParam.scd.requestLength, cdParam.request.length());
						cdParam.request = cdParam.request.substr(0, cdParam.scd.requestLength);
						ret = cdParam.scd.reqHandler(ssmdhParam);
						cdParam.request = fragment;
						cdParam.scd.requestLength = 0;
					} else
						break;
				}
			}

			cdParam.request.clear();
			return ret;
		}
		int onDisconnect(void *param) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(param);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			Rain::tsCout("Info: Client ", Rain::getClientNumIP(*ssmdhParam.cSocket), " disconnected.\r\n");
			fflush(stdout);

			//if receiving mail, process it now if possible
			if (cdParam.cType == "recv" && 
				cdParam.rcd.mailData.length() > 0) {
				Rain::tsCout("Success: Parsed receive mail request from ", Rain::getClientNumIP(*ssmdhParam.cSocket), ".\r\n");
				Rain::tsCout("Info: Forwarding receive mail request to respective recipients.\r\n");
				fflush(stdout);

				//break down the receive request into one for each rcpt to
				for (auto rcpt : cdParam.rcd.rcptTo) {
					//we know one of from or rcpt is @domain
					//if to is @domain, get the real to
					std::string trueTo = rcpt;
					if (Rain::getEmailDomain(trueTo) == (*ccParam.config)["operating-domain"]) {
						trueTo = Rain::strDecodeB64(ccParam.b64Users[Rain::strEncodeB64(Rain::getEmailUser(rcpt))]);
					}

					//create a CSM to send a request to EmiliaMailServer's same port to send this email to the right location
					Rain::ClientSocketManager csm;

					//don't need to verify; just send the information and exit
					csm.setEventHandlers(NULL, NULL, NULL, NULL);
					csm.setClientTarget("localhost", 25, 25);

					//authentication
					Rain::sendBlockMessage(csm, (*ccParam.config)["client-auth"]);
					
					//from address
					Rain::sendBlockMessage(csm, "server@emilia-tan.com");

					//to address
					Rain::sendBlockMessage(csm, trueTo);

					//entire email body, including end
					Rain::sendBlockMessage(csm, cdParam.rcd.mailData);

					csm.blockForMessageQueue();
				}

				Rain::tsCout("Success: Processed eceive mail request from ", Rain::getClientNumIP(*ssmdhParam.cSocket), ".\r\n");
				fflush(stdout);
			} else {
				Rain::tsCout("Failure: Invalid receive mail request from ", Rain::getClientNumIP(*ssmdhParam.cSocket), ".\r\n");
				fflush(stdout);
			}

			delete ssmdhParam.delegateParam;
			return 0;
		}
	}
}