#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

#include "ConnectionHandlers.h"

#include <shellapi.h>
#include <string>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		typedef int(*RequestMethodHandler)(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);

		//specific handlers for different messages
		/*
		Method: authenticate
		Contains:
		password
		*/
		int HRAuthenticate(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		/*
		Method: prod-upload
		Linebreaks: \r\n
		Contains:
		a line specifying the number of files to be transferred, N
		2N lines specifying the relative (to /Production) path of transferred files, and then the length, in bytes, of the file on a separate line
		An empty line (just \r\n) (end of the 'header' block)
		The bytes of each file, in consecutive format
		Blocking: prod-upload might be blocked into multiple blocks; however, each block will have the prod-upload method; assume that they all build off of each other until the end of a single prod-upload request
		*/
		int HRProdUpload(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		/*
		Method: prod-download
		Contains:
		same information as prod-upload
		*/
		int HRProdDownload(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		/*
		*/
		int HRProdStop(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		/*
		*/
		int HRProdStart(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		/*
		*/
		int HRSyncStop(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		/*
		*/
		int HRSyncStart(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
	}
}