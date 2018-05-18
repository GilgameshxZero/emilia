#pragma once
#include "../../Common/RainLibrary3/RainLibraries.h"

#include "ConnectionParams.h"

#include <string>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		typedef int(*RequestMethodHandler)(ConnectionDelegateParam &);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(ConnectionDelegateParam &cdParam);

		//specific handlers for different messages
		int HRAuthenticate(ConnectionDelegateParam &cdParam);
		int HRProdUpload(ConnectionDelegateParam &cdParam);
		int HRProdDownload(ConnectionDelegateParam &cdParam);
		int HRProdStop(ConnectionDelegateParam &cdParam);
		int HRProdStart(ConnectionDelegateParam &cdParam);
		int HRSyncStop(ConnectionDelegateParam &cdParam);
		int HRSyncStart(ConnectionDelegateParam &cdParam);
	}
}