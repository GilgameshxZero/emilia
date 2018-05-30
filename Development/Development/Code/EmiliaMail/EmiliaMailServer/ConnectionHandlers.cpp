#include "ConnectionHandlers.h"

namespace Monochrome3 {
	namespace EmiliaMailServer {
		int onConnect(void *param) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(param);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			ssmdhParam.delegateParam = new ConnectionDelegateParam();

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
						cdParam.recvHandler = HRRecvEhlo;
					} else {
						cdParam.cType = "send";
						cdParam.requestLength = Rain::strToT<std::size_t>(method);
						cdParam.request = cdParam.request.substr(firstSpace + 1, cdParam.request.length());
					}
				}
			}

			int ret = 0;
			if (cdParam.cType == "recv") {
				//test if message is done receiving
				if (cdParam.recvHandler != HRRecvSendMail) {
					//message terminated by CRLF
					if (cdParam.request.substr(cdParam.request.length() - 2, 2) == "\r\n") {
						//message complete, send to handler
						ret = cdParam.recvHandler(ssmdhParam);
					}
				} else {
					//message terminated by \r\n.\r\n
					if (cdParam.request.substr(cdParam.request.length() - 5, 5) == "\r\n.\r\n") {
						//message complete, send to handler
						ret = cdParam.recvHandler(ssmdhParam);
					}
				}
			} else if (cdParam.cType == "send") {
				//message is done if its length is correct
				if (cdParam.requestLength == cdParam.request.length()) {
					ret = HRSendRequest(ssmdhParam);
				}
			}
			return ret;
		}
		int onDisconnect(void *param) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(param);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			delete ssmdhParam.delegateParam;
			return 0;
		}
	}
}