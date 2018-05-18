#pragma once
#include "../../Common/RainLibrary3/RainLibraries.h"

#include "ConnectionParams.h"

#include <shellapi.h>
#include <string>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		typedef int(*RequestMethodHandler)(ConnectionDelegateParam &);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(ConnectionDelegateParam &cdParam);

		//specific handlers for different messages
		int HRAuthenticate(ConnectionDelegateParam &cdParam);
		/*
		Method: authenticate
		Contains:
			password
		*/
		int HRProdUpload(ConnectionDelegateParam &cdParam);
		/*
		Method: prod-upload
		Linebreaks: \r\n
		Contains:
			a line specifying the number of files to be transferred, N
			N lines specifying the relative (to /Production) path of transferred files
			N lines specifying the length, in bytes, of each file in the same order
			An empty line (just \r\n) (end of the 'header' block)
			The bytes of each file, in consecutive format
		Blocking: prod-upload might be blocked into multiple blocks; however, each block will have the prod-upload method; assume that they all build off of each other until the end of a single prod-upload request
		*/
		int HRProdDownload(ConnectionDelegateParam &cdParam);
		/*
		Method: prod-download
		Contains:
			same information as prod-upload
		*/
		int HRProdStop(ConnectionDelegateParam &cdParam);
		/*
		*/
		int HRProdStart(ConnectionDelegateParam &cdParam);
		/*
		*/
		int HRSyncStop(ConnectionDelegateParam &cdParam);
		/*
		*/
		int HRSyncStart(ConnectionDelegateParam &cdParam);
		/*
		*/
	}
}