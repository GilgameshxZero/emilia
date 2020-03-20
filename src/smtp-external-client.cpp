#include "smtp-external-client.hpp"

namespace Emilia {
	namespace SMTPServer {
		int onExternalConnect(void *param) {
			Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(param);
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);

			//unsuccessful at the beginning
			ecParam.status = -1;

			return 0;
		}
		int onExternalMessage(void *param) {
			Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(param);
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);

			int ret = 0;

			//accumulate requests
			//a request is complete when its final line has a space immediately after a few consecutive numbers
			ecParam.request += *csmdhParam.message;
			std::size_t pos = ecParam.request.rfind(Rain::CRLF);
			if (pos == ecParam.request.length() - 2) {
				std::size_t pos2 = ecParam.request.rfind(Rain::CRLF, pos - 1);
				std::string finalLine;
				if (pos2 != ecParam.request.npos) {
					finalLine = ecParam.request.substr(pos2 + 2, pos);
				} else {
					finalLine = ecParam.request.substr(0, pos);
				}

				char firstNonNumer = '-';
				for (char c : finalLine) {
					if (!(c >= '0' && c <= '9')) {
						firstNonNumer = c;
						break;
					}
				}
				if (firstNonNumer == ' ') {
					//request is complete, send it off to the right request handler
					if (ecParam.reqHandler == NULL)  //this shouldn't happen, but there might be multithreading issues that cause this
						ret = -1;
					else
						ret = ecParam.reqHandler(csmdhParam);
					ecParam.request.clear();
				}
			}

			return ret;
		}
		int onExternalDisconnect(void *param) {
			Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(param);
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);

			//mark the event as finished for listeners
			SetEvent(ecParam.hFinish);

			return 0;
		}

		int EHREhlo(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 220)
				return 1;

			// change the domain to something more general
			csmdhParam.csm->sendRawMessage("EHLO gilgamesh.cc" + Rain::CRLF);
			ecParam.reqHandler = EHRMailFrom;
			return 0;
		}
		int EHRMailFrom(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 250)
				return 1;

			csmdhParam.csm->sendRawMessage("MAIL FROM:<" + *ecParam.from + ">" + Rain::CRLF);
			ecParam.reqHandler = EHRRcptTo;
			return 0;
		}
		int EHRRcptTo(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 250)
				return 1;

			csmdhParam.csm->sendRawMessage("RCPT TO:<" + *ecParam.to + ">" + Rain::CRLF);
			ecParam.reqHandler = EHRPreData;
			return 0;
		}
		int EHRPreData(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 250)
				return 1;

			csmdhParam.csm->sendRawMessage("DATA" + Rain::CRLF);
			ecParam.reqHandler = EHRData;
			return 0;
		}
		int EHRData(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 354)
				return 1;

			csmdhParam.csm->sendRawMessage(ecParam.data);
			ecParam.reqHandler = EHRQuit;
			return 0;
		}
		int EHRQuit(Rain::ClientSocketManager::DelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 250)
				return 1;

			csmdhParam.csm->sendRawMessage("QUIT" + Rain::CRLF);
			ecParam.reqHandler = NULL;

			//if we're here, then we've successfully sent the mail
			ecParam.status = 0;

			return 1;
		}
	}  // namespace SMTPServer
}  // namespace Emilia