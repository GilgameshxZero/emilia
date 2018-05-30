#include "ConnectionHandlers.h"

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		static const std::string headerDelim = "\r\n\r\n";

		int onConnect(void *funcParam) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			if (ccParam.clientConnected)
				return 1; //immediately terminate

			ccParam.clientConnected = true;
			Rain::tsCout("Info: client connected.\r\n");
			fflush(stdout);

			//create the delegate parameter for the first time
			ConnectionDelegateParam *cdParam = new ConnectionDelegateParam();
			ssmdhParam.delegateParam = reinterpret_cast<void *>(cdParam);

			//initialize cdParam here
			cdParam->request = "";
			cdParam->requestLength = 0;

			cdParam->authenticated = false;
			return 0;
		}
		int onMessage(void *funcParam) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ssmdhParam.delegateParam);

			//delegate to request handlers once the message is complete
			//message/request length is at the beginning, as a base-10 string, before a space
			cdParam.request += *ssmdhParam.message;

			int ret = 0;
			while (true) {
				if (cdParam.requestLength == 0) {
					std::size_t firstSpace = cdParam.request.find(' ');
					if (firstSpace != std::string::npos) {
						cdParam.requestLength = Rain::strToT<std::size_t>(cdParam.request.substr(0, firstSpace));
						cdParam.request = cdParam.request.substr(firstSpace + 1, cdParam.request.length());
					}
				}

				//if message is complete
				if (cdParam.requestLength != 0 && cdParam.request.length() >= cdParam.requestLength) {
					std::string fragment = cdParam.request.substr(cdParam.requestLength, cdParam.request.length());
					cdParam.request = cdParam.request.substr(0, cdParam.requestLength);

					int hrReturn = HandleRequest(ssmdhParam);
					if (hrReturn < 0 || (hrReturn > 0 && ret >= 0))
						ret = hrReturn;

					cdParam.request = fragment;
					cdParam.requestLength = 0;
				} else
					break;
			}

			return 0;
		}
		int onDisconnect(void *funcParam) {
			Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam = *reinterpret_cast<Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ssmdhParam.callerParam);

			ccParam.clientConnected = false;
			Rain::tsCout("Info: client disconnected.\r\n");
			fflush(stdout);

			//free the delegate parameter
			delete ssmdhParam.delegateParam;
			return 0;
		}
	}
}