#include "smtp-external-client.h"

namespace Emilia {
	namespace SMTPServer {
		int onExternalConnect(void *param) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);

			//unsuccessful at the beginning
			ecParam.status = -1;

			return 0;
		}
		int onExternalMessage(void *param) {
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);

			int ret = 0;

			//accumulate requests
			//a request is complete when its final line has a space immediately after a few consecutive numbers
			ecParam.request += *csmdhParam.message;
			std::size_t pos = ecParam.request.rfind("\r\n");
			if (pos == ecParam.request.length() - 2) {
				std::size_t pos2 = ecParam.request.rfind("\r\n", pos - 1);
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
			Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);

			//mark the event as finished for listeners
			SetEvent(ecParam.hFinish);

			return 0;
		}

		int EHREhlo(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 220)
				return 1;

			csmdhParam.csm->sendRawMessage("EHLO emilia-tan.com\r\n");
			ecParam.reqHandler = EHRMailFrom;
			return 0;
		}
		int EHRMailFrom(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 250)
				return 1;

			csmdhParam.csm->sendRawMessage("MAIL FROM:<");
			csmdhParam.csm->sendRawMessage(ecParam.from);
			csmdhParam.csm->sendRawMessage(">\r\n");
			ecParam.reqHandler = EHRRcptTo;
			return 0;
		}
		int EHRRcptTo(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 250)
				return 1;

			csmdhParam.csm->sendRawMessage("RCPT TO:<");
			csmdhParam.csm->sendRawMessage(ecParam.to);
			csmdhParam.csm->sendRawMessage(">\r\n");
			ecParam.reqHandler = EHRPreData;
			return 0;
		}
		int EHRPreData(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 250)
				return 1;

			csmdhParam.csm->sendRawMessage("DATA\r\n");
			ecParam.reqHandler = EHRData;
			return 0;
		}
		int EHRData(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 354)
				return 1;

			csmdhParam.csm->sendRawMessage(ecParam.data);
			ecParam.reqHandler = EHRQuit;
			return 0;
		}
		int EHRQuit(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam) {
			ExternalConnectionParam &ecParam = *reinterpret_cast<ExternalConnectionParam *>(csmdhParam.delegateParam);
			if (Rain::getSMTPStatus(ecParam.request) != 250)
				return 1;

			csmdhParam.csm->sendRawMessage("QUIT\r\n");
			ecParam.reqHandler = NULL;

			//if we're here, then we've successfully sent the mail
			ecParam.status = 0;

			return 1;
		}
	}  // namespace SMTPServer
}  // namespace Emilia