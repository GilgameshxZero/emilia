#include "smtp-server.h"

namespace Emilia {
	namespace SMTPServer {
		int onConnect(void *param) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(param);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			ccParam.logSMTP->setSocketSrc(ssmdhParam.ssm, true);

			ssmdhParam.delegateParam = new ConnectionDelegateParam();
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);
			Rain::tsCout("[", ++ccParam.connectedClients, "] SMTP Client ", Rain::getClientNumIP(*ssmdhParam.cSocket), " connected.", Rain::CRLF);
			std::cout.flush();

			//in either request type, send a 220
			//in send requests, the 220 will be ignored, but that's fine
			ssmdhParam.ssm->sendRawMessage("220 Emilia is at your service! ^_^" + Rain::CRLF);
			cdParam.rcd.reqHandler = HRREhlo;

			return 0;
		}
		int onMessage(void *param) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(param);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			cdParam.request += *ssmdhParam.message;

			int ret = 0;
			while (true) {
				if (cdParam.rcd.reqHandler != HRRData) {
					//message terminated by CRLF
					std::size_t end = cdParam.request.find(Rain::CRLF);
					if (end != std::string::npos) {
						//message complete, send to handler
						std::string fragment = cdParam.request.substr(end + 2, cdParam.request.length());
						cdParam.request = cdParam.request.substr(0, end + 2);
						ret = cdParam.rcd.reqHandler(ssmdhParam);
						cdParam.request = fragment;
					} else {
						break;
					}
				} else {
					//message terminated by \r\n.\r\n
					std::size_t end = cdParam.request.find(Rain::CRLF + "." + Rain::CRLF);
					if (end != std::string::npos) {
						//message complete, send to handler
						std::string fragment = cdParam.request.substr(end + 5, cdParam.request.length());
						cdParam.request = cdParam.request.substr(0, end + 5);
						ret = cdParam.rcd.reqHandler(ssmdhParam);
						cdParam.request = fragment;
					} else {
						break;
					}
				}
			}

			return ret;
		}
		int onDisconnect(void *param) {
			Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::DelegateHandlerParam *>(param);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			ccParam.logSMTP->setSocketSrc(ssmdhParam.ssm, false);

			Rain::tsCout("[", --ccParam.connectedClients, "] SMTP Client ", Rain::getClientNumIP(*ssmdhParam.cSocket), " disconnected." + Rain::CRLF);
			std::cout.flush();

			//if receiving mail, process it now if possible
			if (cdParam.rcd.mailData.length() > 0) {
				Rain::tsCout("Success: Parsed receive mail request from ", Rain::getClientNumIP(*ssmdhParam.cSocket), "." + Rain::CRLF);
				Rain::tsCout("Forwarding receive mail request to respective recipients." + Rain::CRLF);
				std::cout.flush();

				//refresh userdata
				std::map<std::string, std::string> users = Rain::readParameterFile(ccParam.project + (*ccParam.config)["smtp-users"].s());
				for (auto it : users) {
					ccParam.b64Users[Rain::strEncodeB64(it.first)] = Rain::strEncodeB64(it.second);
				}

				//break down the receive request into one for each rcpt to
				for (auto rcpt : cdParam.rcd.rcptTo) {
					//we know one of from or rcpt is @domain
					//if to is @domain, get the real to
					std::string trueTo = rcpt,
						rcptDomain = Rain::getEmailDomain(rcpt);
					std::set<std::string> acceptedDomains = (*ccParam.config)["smtp-domain"].keys();
					if (acceptedDomains.find(rcptDomain) != acceptedDomains.end()) {
						if (ccParam.b64Users.find(Rain::strEncodeB64(Rain::getEmailUser(rcpt))) == ccParam.b64Users.end()) {
							//if the user doesn't exist, throw away the email
							Rain::tsCout("Failure: Client could not find local user ", rcpt, "; email recipient ignored and email discarded." + Rain::CRLF);
							std::cout.flush();
							continue;
						} else {
							//user exists, so get the email
							trueTo = Rain::strDecodeB64(ccParam.b64Users[Rain::strEncodeB64(Rain::getEmailUser(rcpt))]);
							Rain::tsCout("Forwarding email intended for user ", rcpt, " to ", trueTo, "." + Rain::CRLF);
							std::cout.flush();
						}
					}

					//get DNS information
					std::string emailHostDomain = Rain::getEmailDomain(trueTo);
					std::vector<std::string> smtpServers;
					DNS_RECORD *dnsRecord;
					if (!DnsQuery(emailHostDomain.c_str(), DNS_TYPE_MX, DNS_QUERY_STANDARD, NULL, &dnsRecord, NULL)) {
						DNS_RECORD *curDNSR = dnsRecord;
						while (curDNSR != NULL && curDNSR->wType == DNS_TYPE_MX) {
							smtpServers.push_back(curDNSR->Data.MX.pNameExchange);
							curDNSR = curDNSR->pNext;
						}
						DnsRecordListFree(dnsRecord, DnsFreeRecordList);
						Rain::tsCout("Success: Successfully queried ", smtpServers.size(), " MX DNS record(s) for domain ", emailHostDomain, ". Connecting to SMTP server..." + Rain::CRLF);
						std::cout.flush();
					} else {
						Rain::tsCout("Failure: Failed query for MX DNS record for domain ", emailHostDomain, ". Client will discard email and terminate..." + Rain::CRLF);
						std::cout.flush();
						ssmdhParam.ssm->sendRawMessage("Failure: Failed query for MX DNS record for domain " + emailHostDomain + ". Client will discard email and terminate connection." + Rain::CRLF);
						return 1;
					}

					//use CSM to send the mail and block for results
					Rain::ClientSocketManager csm;
					ExternalConnectionParam ecParam;

					ccParam.logSMTP->setSocketSrc(&csm, true);

					ecParam.hFinish = CreateEvent(NULL, TRUE, FALSE, NULL);
					ecParam.reqHandler = EHREhlo;
					ecParam.to = &trueTo;
					ecParam.from = &cdParam.rcd.mailFrom;
					ecParam.data = &cdParam.rcd.mailData;
					ecParam.ehloDomain = *(*ccParam.config).keys().begin();

					csm.setEventHandlers(onExternalConnect, onExternalMessage, onExternalDisconnect, &ecParam);

					//attempt to connect to each of the MX records in order, each with a timeout, until all of them fail or one of them succeeds
					bool connected = false;
					for (std::string server : smtpServers) {
						csm.setClientTarget(server, 25, 25);

						//only allow connecting to client for a timeout before discarding email
						csm.blockForConnect((*ccParam.config)["smtp-to"].i());
						if (csm.getSocketStatus() != csm.STATUS_CONNECTED) {
							Rain::tsCout("Failure: Could not connect to SMTP server ", server, ". Client will try next MX SMTP server if available..." + Rain::CRLF);
							std::cout.flush();
						} else {
							Rain::tsCout("Success: Connected to SMTP server ", server, ". Sending email..." + Rain::CRLF);
							std::cout.flush();
							connected = true;
							break;
						}
					}

					if (!connected) {
						Rain::tsCout("Failure: All MX record SMTP servers failed to connect. Client will discard email and terminate." + Rain::CRLF);
						std::cout.flush();
						ssmdhParam.ssm->sendRawMessage("-3");
						return 1;
					}

					//block for communications, which the event handlers will send
					if (ecParam.hFinish == NULL) {
						Rain::reportError(GetLastError(), "ecParam.hFinish event failed to be created. Aborting connection...");
						return 1;
					}
					WaitForSingleObject(ecParam.hFinish, INFINITE);
					CloseHandle(ecParam.hFinish);

					ccParam.logSMTP->setSocketSrc(&csm, false);
				}

				Rain::tsCout("Finished processing receive mail request from ", Rain::getClientNumIP(*ssmdhParam.cSocket), "." + Rain::CRLF);
			} else {
				Rain::tsCout("Failure: Invalid mail request from ", Rain::getClientNumIP(*ssmdhParam.cSocket), "." + Rain::CRLF);
			}

			std::cout.flush();
			delete ssmdhParam.delegateParam;
			return 0;
		}

		int HRREhlo(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			std::string command = cdParam.request.substr(0, 4);

			//we don't want to deal with HELO commands, which are not RFC compliant anyway
			if (command == "HELO") {
				ssmdhParam.ssm->sendRawMessage("500 Emilia doesn't accept HELO, only EHLO" + Rain::CRLF);
				return 0;
			}

			ssmdhParam.ssm->sendRawMessage("250-Emilia is best girl!" + Rain::CRLF);
			ssmdhParam.ssm->sendRawMessage("250 AUTH LOGIN" + Rain::CRLF);
			cdParam.rcd.reqHandler = HRRPreData;

			return 0;
		}
		int HRRPreData(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			//process commands before DATA
			std::string command = cdParam.request.substr(0, 4);
			if (command == "DATA") {
				ssmdhParam.ssm->sendRawMessage("354 Emilia ready for data" + Rain::CRLF);
				cdParam.rcd.reqHandler = HRRData;
			} else if (command == "AUTH") {
				return HRRAuthLogin(ssmdhParam);
			} else if (command == "MAIL") {
				return HRRMailFrom(ssmdhParam);
			} else if (command == "RCPT") {
				return HRRRcptTo(ssmdhParam);
			} else {
				ssmdhParam.ssm->sendRawMessage("502 Emilia hasn't learned this yet" + Rain::CRLF);
			}

			return 0;
		}
		int HRRData(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			cdParam.rcd.mailData = cdParam.request;

			//check if everything looks right and prepare to close connection
			//one of the email addresses in rcpt or from must be @domain
			bool isAtDomain = false;
			std::set<std::string> acceptedDomains = (*ccParam.config)["smtp-domain"].keys();
			if (acceptedDomains.find(Rain::getEmailDomain(cdParam.rcd.mailFrom)) != acceptedDomains.end()) {
				isAtDomain = true;
			} else {
				bool allAtDomain = true;
				for (auto it : cdParam.rcd.rcptTo) {
					if (acceptedDomains.find(Rain::getEmailDomain(it)) == acceptedDomains.end()) {
						allAtDomain = false;
						break;
					}
				}
				isAtDomain = allAtDomain;
			}

			//we are willing to send email to and from anybody
			if (true || isAtDomain) {
				ssmdhParam.ssm->sendRawMessage("250 OK" + Rain::CRLF);
			} else {
				ssmdhParam.ssm->sendRawMessage("510 Emilia doesn't like talking for strangers (one of either 'RCPT TO' or 'MAIL FROM' must be at the local domain" + Rain::CRLF);
				cdParam.rcd.mailData.clear();
			}

			return 1;
		}

		int HRRAuthLogin(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			if (cdParam.request.substr(0, 10) != "AUTH LOGIN") {
				ssmdhParam.ssm->sendRawMessage("500 Emilia doesn't understand AUTH request" + Rain::CRLF);
				return 0;
			}

			cdParam.request = Rain::strTrimWhite(cdParam.request.substr(10, cdParam.request.length()));
			if (cdParam.request.length() > 0) {
				//might be sending login request username in first go
				return HRRAuthLoginUsername(ssmdhParam);
			} else {
				ssmdhParam.ssm->sendRawMessage("334 VXNlcm5hbWU6" + Rain::CRLF);
				cdParam.rcd.reqHandler = HRRAuthLoginUsername;
			}
			return 0;
		}
		int HRRAuthLoginUsername(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			Rain::strTrimWhite(&cdParam.request);

			//don't check if user exists here, wait until password to hide information
			cdParam.rcd.b64User = cdParam.request;
			ssmdhParam.ssm->sendRawMessage("334 UGFzc3dvcmQ6" + Rain::CRLF);
			cdParam.rcd.reqHandler = HRRAuthLoginPassword;

			return 0;
		}
		int HRRAuthLoginPassword(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			Rain::strTrimWhite(&cdParam.request);

			//refresh userdata
			std::map<std::string, std::string> users = Rain::readParameterFile(ccParam.project + (*ccParam.config)["smtp-users"].s());
			for (auto it : users) {
				ccParam.b64Users[Rain::strEncodeB64(it.first)] = Rain::strEncodeB64(it.second);
			}

			//check if userdata exists
			auto it = ccParam.b64Users.find(cdParam.rcd.b64User);
			if (it != ccParam.b64Users.end() &&
				it->second == cdParam.request) {
				ssmdhParam.ssm->sendRawMessage("235 Emilia authenticated you" + Rain::CRLF);
			} else {
				ssmdhParam.ssm->sendRawMessage("530 Emilia could not authenticate you" + Rain::CRLF);
				cdParam.rcd.b64User.clear();
			}
			cdParam.rcd.reqHandler = HRRPreData;

			return 0;
		}
		int HRRMailFrom(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			if (cdParam.request.substr(0, 9) != "MAIL FROM" || cdParam.request.length() <= 10) {
				ssmdhParam.ssm->sendRawMessage("500 Emilia doesn't understand MAIL request" + Rain::CRLF);
				return 0;
			}

			//gets after the colon, between the brackets
			std::string afterColon = Rain::strTrimWhite(cdParam.request.substr(10, cdParam.request.length()));
			std::size_t b1 = afterColon.find("<"),
				b2 = afterColon.rfind(">");
			if (b1 == std::string::npos || b2 == std::string::npos) {
				ssmdhParam.ssm->sendRawMessage("500 Emilia doesn't understand MAIL request" + Rain::CRLF);
				return 0;
			}
			cdParam.rcd.mailFrom = Rain::strTrimWhite(afterColon.substr(b1 + 1, b2 - b1 - 1));

			//TODO: if sending from current domain, need to be authenticated, unless it's from server@emilia-tan.com, which is publicly accessible without password
			std::set<std::string> acceptedDomains = (*ccParam.config)["smtp-domain"].keys();
			std::string mailFromUser = Rain::getEmailUser(cdParam.rcd.mailFrom),
				mailFromDomain = Rain::getEmailDomain(cdParam.rcd.mailFrom);
			if (acceptedDomains.find(mailFromDomain) != acceptedDomains.end() &&
				cdParam.rcd.b64User.length() == 0 &&
				mailFromUser != "server") {
				ssmdhParam.ssm->sendRawMessage("502 Emilia hasn't authenticated you to do that (sending from a managed domain requires authentication)" + Rain::CRLF);
			} else
				ssmdhParam.ssm->sendRawMessage("250 OK" + Rain::CRLF);
			return 0;
		}
		int HRRRcptTo(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhParam) {
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			if (cdParam.request.substr(0, 7) != "RCPT TO" || cdParam.request.length() <= 8) {
				ssmdhParam.ssm->sendRawMessage("500 Emilia doesn't understand RCPT request" + Rain::CRLF);
				return 0;
			}

			//gets after the colon, between the brackets
			std::string afterColon = Rain::strTrimWhite(cdParam.request.substr(8, cdParam.request.length()));
			std::size_t b1 = afterColon.find("<"),
				b2 = afterColon.rfind(">");
			if (b1 == std::string::npos || b2 == std::string::npos) {
				ssmdhParam.ssm->sendRawMessage("500 Emilia doesn't understand RCPT request" + Rain::CRLF);
				return 0;
			}
			cdParam.rcd.rcptTo.insert(Rain::strTrimWhite(afterColon.substr(b1 + 1, b2 - b1 - 1)));

			ssmdhParam.ssm->sendRawMessage("250 OK" + Rain::CRLF);
			return 0;
		}
	}  // namespace SMTPServer
}  // namespace Emilia