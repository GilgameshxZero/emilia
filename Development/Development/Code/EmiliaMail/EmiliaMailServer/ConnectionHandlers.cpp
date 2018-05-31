#include "ConnectionHandlers.h"

namespace Monochrome3 {
	namespace EmiliaMailServer {
		int onConnect(void *param) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(param);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			ccParam.logger->setSocketSrc(ssmdhParam.ssm, true);

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
						cdParam.scd.requestLength <= cdParam.request.length()) {
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

			ccParam.logger->setSocketSrc(ssmdhParam.ssm, false);

			Rain::tsCout("Info: Client ", Rain::getClientNumIP(*ssmdhParam.cSocket), " disconnected.\r\n");
			fflush(stdout);

			//if receiving mail, process it now if possible
			if (cdParam.cType == "recv" && 
				cdParam.rcd.mailData.length() > 0) {
				Rain::tsCout("Success: Parsed receive mail request from ", Rain::getClientNumIP(*ssmdhParam.cSocket), ".\r\n");
				Rain::tsCout("Info: Forwarding receive mail request to respective recipients.\r\n");
				fflush(stdout);

				//different csm's used to send requests to internal client
				std::vector<Rain::ClientSocketManager> vCSM;
				std::vector<InternalConnectionParam *> vICParam;

				//break down the receive request into one for each rcpt to
				for (auto rcpt : cdParam.rcd.rcptTo) {
					//we know one of from or rcpt is @domain
					//if to is @domain, get the real to
					std::string trueTo = rcpt;
					if (Rain::getEmailDomain(trueTo) == (*ccParam.config)["operating-domain"]) {
						trueTo = Rain::strDecodeB64(ccParam.b64Users[Rain::strEncodeB64(Rain::getEmailUser(rcpt))]);
					}

					//create a CSM to send a request to EmiliaMailServer's same port to send this email to the right location
					vCSM.push_back(Rain::ClientSocketManager());
					vICParam.push_back(new InternalConnectionParam());
					vICParam.back()->hFinish = CreateEvent(NULL, TRUE, FALSE, NULL);
					vICParam.back()->toAddress = trueTo;

					//don't need to verify; just send the information and exit
					vCSM.back().setEventHandlers(onInternalConnect, onInternalMessage, onInternalDisconnect, vICParam.back());
					vCSM.back().setClientTarget("localhost", 25, 25);

					//authentication
					Rain::sendBlockMessage(vCSM.back(), (*ccParam.config)["client-auth"]);
					
					//from address
					Rain::sendBlockMessage(vCSM.back(), "server@emilia-tan.com");

					//to address
					Rain::sendBlockMessage(vCSM.back(), trueTo);

					//entire email body, including end
					Rain::sendBlockMessage(vCSM.back(), cdParam.rcd.mailData);
				}

				//wait on all CSMs to return with a code (and, thus report their success to the console)
				while (vICParam.size() > 0) {
					WaitForSingleObject(vICParam.back()->hFinish, INFINITE);
					CloseHandle(vICParam.back()->hFinish);
					delete vICParam.back();
					vICParam.pop_back();
				}

				Rain::tsCout("Info: Finished processing receive mail request from ", Rain::getClientNumIP(*ssmdhParam.cSocket), ".\r\n");
				fflush(stdout);
			} else {
				Rain::tsCout("Failure: Invalid receive mail request from ", Rain::getClientNumIP(*ssmdhParam.cSocket), ".\r\n");
				fflush(stdout);
			}

			delete ssmdhParam.delegateParam;
			return 0;
		}

		int HRREhlo(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			ssmdhParam.ssm->sendRawMessage("250-Emilia is best girl!\r\n");
			ssmdhParam.ssm->sendRawMessage("250 AUTH LOGIN\r\n");
			cdParam.rcd.reqHandler = HRRPreData;

			return 0;
		}
		int HRRPreData(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			//process commands before DATA
			std::string command = cdParam.request.substr(0, 4);
			if (command == "DATA") {
				ssmdhParam.ssm->sendRawMessage("354 Emilia ready for data\r\n");
				cdParam.rcd.reqHandler = HRRData;
			} else if (command == "AUTH") {
				return HRRAuthLogin(ssmdhParam);
			} else if (command == "MAIL") {
				return HRRMailFrom(ssmdhParam);
			} else if (command == "RCPT") {
				return HRRRcptTo(ssmdhParam);
			} else {
				ssmdhParam.ssm->sendRawMessage("502 Emilia hasn't learned this yet\r\n");
			}

			return 0;
		}
		int HRRData(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			cdParam.rcd.mailData = cdParam.request;

			//check if everything looks right and prepare to close connection
			//one of the email addresses in rcpt or from must be @domain
			bool isAtDomain = false;
			if (Rain::getEmailDomain(cdParam.rcd.mailFrom) == (*ccParam.config)["operating-domain"])
				isAtDomain = true;
			if (!isAtDomain) {
				bool allAtDomain = true;
				for (auto it : cdParam.rcd.rcptTo) {
					if (Rain::getEmailDomain(it) != (*ccParam.config)["operating-domain"]) {
						allAtDomain = false;
						break;
					}
				}
				isAtDomain = allAtDomain;
			}

			if (isAtDomain) {
				ssmdhParam.ssm->sendRawMessage("250 OK\r\n");
			} else {
				ssmdhParam.ssm->sendRawMessage("510 Emilia doesn't like talking for strangers\r\n");
				cdParam.rcd.mailData.clear();
			}

			return 1;
		}

		int HRRAuthLogin(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			if (cdParam.request.substr(0, 10) != "AUTH LOGIN") {
				ssmdhParam.ssm->sendRawMessage("500 Emilia doesn't understand AUTH request\r\n");
				return 0;
			}

			cdParam.request = Rain::strTrimWhite(cdParam.request.substr(10, cdParam.request.length()));
			if (cdParam.request.length() > 0) {
				//might be sending login request username in first go
				return HRRAuthLoginUsername(ssmdhParam);
			} else {
				ssmdhParam.ssm->sendRawMessage("334 VXNlcm5hbWU6\r\n");
				cdParam.rcd.reqHandler = HRRAuthLoginUsername;
			}
			return 0;
		}
		int HRRAuthLoginUsername(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			Rain::strTrimWhite(&cdParam.request);

			//don't check if user exists here, wait until password to hide information
			cdParam.rcd.b64User = cdParam.request;
			ssmdhParam.ssm->sendRawMessage("334 UGFzc3dvcmQ6\r\n");
			cdParam.rcd.reqHandler = HRRAuthLoginPassword;

			return 0;
		}
		int HRRAuthLoginPassword(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			Rain::strTrimWhite(&cdParam.request);

			//refresh userdata
			std::map<std::string, std::string> users = Rain::readParameterFile((*ccParam.config)["dyn-path"] + (*ccParam.config)["dyn-users"]);
			for (auto it : users) {
				ccParam.b64Users[Rain::strEncodeB64(it.first)] = Rain::strEncodeB64(it.second);
			}

			//check if userdata exists
			auto it = ccParam.b64Users.find(cdParam.rcd.b64User);
			if (it != ccParam.b64Users.end() &&
				it->second == cdParam.request) {
				ssmdhParam.ssm->sendRawMessage("235 Emilia authentication success\r\n");
			} else {
				ssmdhParam.ssm->sendRawMessage("530 Emilia could not authenticate the user\r\n");
				cdParam.rcd.b64User.clear();
			}
			cdParam.rcd.reqHandler = HRRPreData;

			return 0;
		}
		int HRRMailFrom(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			if (cdParam.request.substr(0, 9) != "MAIL FROM") {
				ssmdhParam.ssm->sendRawMessage("500 Emilia doesn't understand MAIL request\r\n");
				return 0;
			}

			//gets after the colon
			cdParam.rcd.mailFrom = Rain::strTrimWhite(cdParam.request.substr(10, cdParam.request.length()));

			//if sending from current domain, need to be authenticated
			if (Rain::getEmailDomain(cdParam.rcd.mailFrom) == (*ccParam.config)["operating-domain"] &&
				cdParam.rcd.b64User.length() == 0) {
				ssmdhParam.ssm->sendRawMessage("502 Emilia hasn't authenticated you to do that\r\n");
			} else
				ssmdhParam.ssm->sendRawMessage("250 OK\r\n");
			return 0;
		}
		int HRRRcptTo(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			if (cdParam.request.substr(0, 7) != "RCPT TO") {
				ssmdhParam.ssm->sendRawMessage("500 Emilia doesn't understand RCPT request\r\n");
				return 0;
			}

			//gets after the colon
			cdParam.rcd.rcptTo.insert(Rain::strTrimWhite(cdParam.request.substr(8, cdParam.request.length())));

			ssmdhParam.ssm->sendRawMessage("250 OK\r\n");
			return 0;
		}

		int HRSAuth(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			//all requests to the client come here

			if (cdParam.request != (*ccParam.config)["client-auth"])
				return 1;

			cdParam.scd.reqHandler = HRSFrom;
			return 0;
		}
		int HRSFrom(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			cdParam.scd.from = cdParam.request;
			return 0;
		}
		int HRSTo(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			cdParam.scd.to = cdParam.request;
			return 0;
		}
		int HRSBody(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			cdParam.scd.data = cdParam.request;

			//get DNS information
			std::string emailHostDomain = Rain::getEmailDomain(cdParam.scd.to);
			DNS_RECORD *dnsRecord;
			DnsQuery(emailHostDomain.c_str(), DNS_TYPE_MX, DNS_QUERY_STANDARD, NULL, &dnsRecord, NULL);
			std::string smtpServer = dnsRecord->Data.MX.pNameExchange;
			DnsRecordListFree(dnsRecord, DnsFreeRecordList);

			//use CSM to send the mail and block for results
			Rain::ClientSocketManager csm;
			ExternalConnectionParam ecParam;

			ccParam.logger->setSocketSrc(&csm, true);

			ecParam.hFinish = CreateEvent(NULL, TRUE, FALSE, NULL);
			ecParam.reqHandler = EHREhlo;
			ecParam.to = &cdParam.scd.to;
			ecParam.from = &cdParam.scd.from;
			ecParam.data = &cdParam.scd.data;

			csm.setEventHandlers(onExternalConnect, onExternalMessage, onExternalDisconnect, &ecParam);
			csm.setClientTarget(smtpServer, 25, 25);

			//block for communications
			WaitForSingleObject(ecParam.hFinish, INFINITE);
			CloseHandle(ecParam.hFinish);

			ccParam.logger->setSocketSrc(&csm, false);

			//output a status code and terminate the connection
			//0 is success
			ssmdhParam.ssm->sendRawMessage(Rain::tToStr(ecParam.status));

			return 1;
		}
	}
}