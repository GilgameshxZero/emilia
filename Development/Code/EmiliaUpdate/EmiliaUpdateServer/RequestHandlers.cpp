#include "RequestHandlers.h"

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		int HandleRequest(ConnectionDelegateParam &cdParam) {
			static const std::map<std::string, RequestMethodHandler> methodHandlerMap{
				{"authenticate", HRAuthenticate}, //validates a socket connection session
				{"prod-upload", HRProdUpload}, //updates server-side production files with client files
				{"prod-download", HRProdDownload}, //request for current production files to client
				{"prod-stop", HRProdStop}, //stop all server services
				{"prod-start", HRProdStart}, //start all server services
				{"sync-stop", HRSyncStop}, //stops constant updates and enables other commands
				{"sync-start", HRSyncStart}, //request constant updates for changed production files; disables other commands except sync-stop
			};

			cdParam.requestMethod = cdParam.request.substr(0, cdParam.request.find(' '));
			cdParam.request = cdParam.request.substr(cdParam.request.find(' ') + 1, cdParam.request.length());

			auto handler = methodHandlerMap.find(cdParam.requestMethod);
			if (handler != methodHandlerMap.end())
				return handler->second(cdParam);
			return 0;
		}

		int HRAuthenticate(ConnectionDelegateParam &cdParam) {
			if (cdParam.config->at("client-auth-pass") != cdParam.request) {
				Rain::sendText(*cdParam.cSocket, "Authentication failed.");
			}
		}
		int HRProdUpload(ConnectionDelegateParam &cdParam) {

		}
		int HRProdDownload(ConnectionDelegateParam &cdParam) {

		}
		int HRProdStop(ConnectionDelegateParam &cdParam) {

		}
		int HRProdStart(ConnectionDelegateParam &cdParam) {

		}
		int HRSyncStop(ConnectionDelegateParam &cdParam) {

		}
		int HRSyncStart(ConnectionDelegateParam &cdParam) {

		}
	}
}