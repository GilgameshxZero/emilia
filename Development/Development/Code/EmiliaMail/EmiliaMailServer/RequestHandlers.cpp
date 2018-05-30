#include "RequestHandlers.h"

namespace Monochrome3 {
	namespace EmiliaMailServer {
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
			} else if (command == "AUTH")  {
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

			//use CSM to send the mail

			return 1;
		}
	}
}