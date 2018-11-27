#pragma once

#include "rain-aeternum/rain-libraries.h"

#include <map>
#include <set>
#include <shellapi.h>

namespace Emilia {
	namespace UpdateServer {
		typedef int(*RequestMethodHandler)(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &);

		struct ConnectionCallerParam {
			//global config options
			std::map<std::string, std::string> *config;

			//event to break the input loop in main
			HANDLE hInputExitEvent;

			//whether a client is connected; only allow one client to connect at a time
			bool clientConnected;
		};

		struct ConnectionDelegateParam {
			//accumulated request from messages
			std::string request;

			//length of the request
			std::size_t requestLength;

			//parsed first section of the request
			std::string requestMethod;

			//whether current socket is authenticated
			bool authenticated;
		};

		//handlers for RecvThread
		int onConnect(void *funcParam);
		int onMessage(void *funcParam);
		int onDisconnect(void *funcParam);

		//general method which takes request and distributes it to appropriate method handler
		int HandleRequest(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);

		//specific handlers for different messages
		int HRAuthenticate(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRPush(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRPushExclusive(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRPull(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRSync(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRStart(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		int HRStop(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);

		///*
		//Note: Only uploads/downloads files, not empty dirs
		//Method: prod-upload
		//Linebreaks: \r\n
		//Contains:
		//a line specifying the number of files to be transferred, N
		//2N lines specifying the relative (to /Production) path of transferred files, and then the length, in bytes, of the file on a separate line
		//An empty line (just \r\n) (end of the 'header' block)
		//The bytes of each file, in consecutive format
		//Blocking: prod-upload might be blocked into multiple blocks; however, each block will have the prod-upload method; assume that they all build off of each other until the end of a single prod-upload request
		//*/
		//int HRProdUpload(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		///*
		//Note: Only uploads/downloads files, not empty dirs
		//Method: prod-download
		//Contains:
		//same information as prod-upload
		//*/
		//int HRProdDownload(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		///*
		//Method: prod-download
		//Contains: nothing
		//*/
		//int HRProdStop(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		///*
		//Method: prod-stop
		//Contains: nothing
		//*/
		//int HRProdStart(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		///*
		//Method: prod-start
		//Contains: nothing
		//*/
		//int HRSyncStop(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
		///*
		//*/
		//int HRSyncStart(Rain::ServerSocketManager::ServerSocketManagerDelegateHandlerParam &ssmdhParam);
	}
}