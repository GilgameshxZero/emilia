#include "ConnectionHandlers.h"

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		static const std::string headerDelim = "\r\n\r\n";

		void onConnectionInit(void *funcParam) {
			Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam = *reinterpret_cast<Rain::WSA2ListenThreadRecvFuncDelegateParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);

			//create the delegate parameter for the first time
			ConnectionDelegateParam *cdParam = new ConnectionDelegateParam();
			ltrfdParam.delegateParam = reinterpret_cast<void *>(cdParam);

			//initialize cdParam here
			cdParam->request = "";
			cdParam->requestLength = 0;
		}
		void onConnectionExit(void *funcParam) {
			//free the delegate parameter
			delete reinterpret_cast<Rain::WSA2ListenThreadRecvFuncDelegateParam *>(funcParam)->delegateParam;
		}
		int onConnectionProcessMessage(void *funcParam) {
			Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam = *reinterpret_cast<Rain::WSA2ListenThreadRecvFuncDelegateParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ltrfdParam.delegateParam);

			//delegate to request handlers once the message is complete
			//message/request length is at the beginning, as a base-10 string, before a space
			cdParam.request += ltrfdParam.message;
			if (cdParam.requestLength != 0) {
				std::size_t firstSpace = cdParam.request.find(' ');
				if (firstSpace != std::string::npos) {
					cdParam.requestLength = Rain::strToT<std::size_t>(cdParam.request.substr(0, firstSpace));
					cdParam.request = cdParam.request.substr(firstSpace + 1, cdParam.request.length());
				}
			}

			//if message is complete
			if (cdParam.requestLength != 0 && cdParam.request.length() == cdParam.requestLength) {
				return HandleMessage(ltrfdParam);
			}

			//< 0 is error, 0 is keep-alive, and > 0 is peacefully close
			return 1;
		}
	}
}