#pragma once

#include "rain-aeternum/rain-libraries.h"

#include <map>
#include <set>

namespace Emilia {
	namespace UpdateClient {
		typedef int(*RequestMethodHandler)(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &);

		struct ConnectionHandlerParam {
			//global config
			std::map<std::string, std::string> *config;

			//accumulated request
			std::string request;

			//length of the request
			std::size_t requestLength;

			//parsed first section of the request
			std::string requestMethod;

			//a variable, manually set by the CSM thread, which will trigger an event when it reaches 0
			//decreases by 1 every time a response is complete, or by a trigger specific to each method
			int waitingRequests;

			//event triggered by waitingRequests
			HANDLE doneWaitingEvent;

			//the success of the last request, as a code
			//nonzero for error
			int lastSuccess;
		};

		//handlers for ClientSocketManager
		int onConnect(void *funcParam);
		int onDisconnect(void *funcParam);
		int onMessage(void *funcParam);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam);

		//specific handlers for different responses to requests sent by the client
		int HRAuthenticate(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam);
		int HRPush(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam);
		int HRPushExclusive(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam);
		int HRPull(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam);
		int HRSync(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam);
		int HRStart(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam);
		int HRStop(Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhParam);

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
	}
}