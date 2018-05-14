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
		}
		void onConnectionExit(void *funcParam) {
			//free the delegate parameter
			delete reinterpret_cast<Rain::WSA2ListenThreadRecvFuncDelegateParam *>(funcParam)->delegateParam;
		}
		int onConnectionProcessMessage(void *funcParam) {
			Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam = *reinterpret_cast<Rain::WSA2ListenThreadRecvFuncDelegateParam *>(funcParam);
			ConnectionCallerParam &ccParam = *reinterpret_cast<ConnectionCallerParam *>(ltrfdParam.callerParam);
			ConnectionDelegateParam &cdParam = *reinterpret_cast<ConnectionDelegateParam *>(ltrfdParam.delegateParam);

			

			//< 0 is error, 0 is keep-alive, and > 0 is peacefully close
			return 1;
		}
	}
}