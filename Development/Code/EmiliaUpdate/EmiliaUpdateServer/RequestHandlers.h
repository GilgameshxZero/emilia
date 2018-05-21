#pragma once
#include "../../Common/RainLibrary3/RainLibraries.h"

#include "ConnectionCallerParam.h"
#include "ConnectionDelegateParam.h"

#include <shellapi.h>
#include <string>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		typedef int(*RequestMethodHandler)(Rain::WSA2ListenThreadRecvFuncDelegateParam &);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam);

		//specific handlers for different messages
		int HRAuthenticate(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam);
		/*
		Method: authenticate
		Contains:
			password
		*/
		int HRProdUpload(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam);
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
		int HRProdDownload(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam);
		/*
		Method: prod-download
		Contains:
			same information as prod-upload
		*/
		int HRProdStop(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam);
		/*
		*/
		int HRProdStart(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam);
		/*
		*/
		int HRSyncStop(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam);
		/*
		*/
		int HRSyncStart(Rain::WSA2ListenThreadRecvFuncDelegateParam &ltrfdParam);
		/*
		*/
	}
}