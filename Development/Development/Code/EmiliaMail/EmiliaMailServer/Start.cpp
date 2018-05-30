#include "Start.h"

namespace Monochrome3 {
	namespace EmiliaMailServer {
		int start(int argc, char* argv[]) {
			//parameters
			std::map<std::string, std::string> config;

			std::string configLocFile = "config-loc.ini";
			Rain::concatMap(config, Rain::readParameterFile(configLocFile));
			std::string configFile = config["config-path"] + config["config-file"];
			Rain::concatMap(config, Rain::readParameterFile(configFile));
			std::string authenticationFile = config["config-path"] + config["auth-file"];
			Rain::concatMap(config, Rain::readParameterFile(authenticationFile));

			std::vector<std::string> prodServers;
			prodServers = Rain::readMultilineFile(config["config-path"] + config["prod-servers"]);

			//debugging
			Rain::createDirRec(config["aux-path"]);
			Rain::redirectCerrFile(config["aux-path"] + config["aux-error"], true);
			HANDLE hFMemLeak = Rain::logMemoryLeaks(config["aux-path"] + config["aux-memory"]);

			Rain::LogStream logger;
			logger.setFileDst(config["aux-path"] + config["aux-log"], true);
			logger.setStdHandleSrc(STD_OUTPUT_HANDLE, true);

			//output parameters
			Rain::tsCout("Starting...\r\n" + Rain::tToStr(config.size()) + " configuration options:\r\n");
			for (std::map<std::string, std::string>::iterator it = config.begin(); it != config.end(); it++)
				Rain::tsCout("\t" + it->first + ": " + it->second + "\r\n");

			Rain::tsCout("\r\nCommand line arguments: " + Rain::tToStr(argc) + "\r\n\r\n");
			for (int a = 0; a < argc; a++) {
				Rain::tsCout(std::string(argv[a]) + "\r\n");
			}
			Rain::tsCout("\r\n");

			//set up network stuff to continuously accept sockets
			ConnectionCallerParam ccParam;
			ccParam.config = &config;
			ccParam.hInputExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			ccParam.serverStatus.resize(prodServers.size());
			ccParam.clientConnected = false;
			for (int a = 0; a < prodServers.size(); a++) {
				ccParam.serverStatus[a].path = Rain::pathToAbsolute(config["prod-root-dir"] + prodServers[a]);
				ccParam.serverStatus[a].status = "stopped";
			}

			Rain::ServerManager sm;
			sm.setEventHandlers(onConnectionInit, onConnectionProcessMessage, onConnectionExit, &ccParam);
			sm.setRecvBufLen(Rain::strToT<std::size_t>(config["recv-buflen"]));
			if (!sm.setServerListen(Rain::strToT<DWORD>(config["ports-low"]), Rain::strToT<DWORD>(config["ports-high"]))) {
				Rain::tsCout("Server listening on port ", sm.getListeningPort(), "..\r\n");
			} else {
				Rain::tsCout("Fatal error: could not setup server listening.\r\n");
				DWORD error = GetLastError();
				Rain::reportError(error, "Fatal error: could not setup server listening.");
				logger.setStdHandleSrc(STD_OUTPUT_HANDLE, false);
				WSACleanup();
				if (hFMemLeak != NULL)
					CloseHandle(hFMemLeak);
				return error;
			}

			//setup listening on the port
			WSADATA wsaData;
			struct addrinfo *sAddr;
			SOCKET lSocket;
			if (Rain::quickServerInit(wsaData, config["smtpPort"], &sAddr, lSocket)) {
				Rain::reportError(GetLastError(), "error in quickServerInit");
				return -1;
			}
			if (Rain::listenServSocket(lSocket)) {
				Rain::reportError(GetLastError(), "error in listenServSocket");
				return -1;
			}
			std::cout << "Listening...\r\n";
			Rain::fastOutputFile(config["logFile"], "Listening...\r\n", true);

			//similar to EMTServer setup
			//main thread responsible for capturing cin and commands
			//spawn listening threads to deal with socket connections
			//arrange threads in a linked list, with dummy beginning and end nodes
			//ltParam needs to be dynamically allocated, because it will be freed by listenThread
			//however, the mutex should be allocated outside ltParam, because we will need to use it again even after the threads have exited
			std::mutex ltLLMutex;

			std::mutex smtpClientMutex;

			ListenThreadParam *ltParam = new ListenThreadParam();
			ltParam->lSocket = &lSocket;
			ltParam->ltLLMutex = &ltLLMutex;
			ltParam->smtpClientMutex = &smtpClientMutex;
			ltParam->config = &config;

			//create dummy head and tail nodes for the ListenThreadParam linked list, and put the new LTP in between
			ListenThreadParam LTPLLHead, LTPLLTail;
			LTPLLHead.prevLTP = NULL;
			LTPLLHead.nextLTP = ltParam;
			ltParam->prevLTP = &LTPLLHead;
			ltParam->nextLTP = &LTPLLTail;
			LTPLLTail.prevLTP = ltParam;
			LTPLLTail.nextLTP = NULL;

			CreateThread(NULL, 0, listenThread, reinterpret_cast<LPVOID>(ltParam), 0, NULL);

			//at this point, we should assume that ltParam may be deleted already

			//wait for commands
			while (true) {
				static std::string command, tmp;
				std::cout << "Accepting commands...\r\n";
				std::cin >> command;
				std::getline(std::cin, tmp);

				if (command == "exit") {
					//first, close the listening socket so that all calls to 'accept' terminate
					closesocket(lSocket);
					std::cout << "Socket listen closed\r\n";
					Rain::fastOutputFile(config["logFile"], "Socket listen closed\r\n", true);

					//shutdown threads by walking through the linked list and sending the end message
					//while walking through linked list, keep track of any thread handles
					std::vector<HANDLE> hThreads;

					ltLLMutex.lock();
					ListenThreadParam *curLTP = LTPLLHead.nextLTP;
					while (curLTP != &LTPLLTail) {
						hThreads.push_back(curLTP->hThread);

						//use postmessage here because we want the thread of the window to process the message, allowing destroywindow to be called
						//WM_RAINAVAILABLE + 1 is the end message
						PostMessage(curLTP->rainWnd.hwnd, WM_LISTENWNDEND, 0, 0);
						curLTP = curLTP->nextLTP;
					}
					ltLLMutex.unlock();

					std::cout << "Found " << hThreads.size() << " active ListenThreads\r\n";
					Rain::fastOutputFile(config["logFile"], "Found " + Rain::tToStr(hThreads.size()) + " active ListenThreads\r\n", true);

					//join all handles previously found in the linked list, waiting for all the threads to terminate
					//no need to close thread handles, GetCurrentThread doesn't require that
					for (HANDLE hThread : hThreads)
						WaitForSingleObject(hThread, 0);

					std::cout << "All threads finished and joined\r\n";
					Rain::fastOutputFile(config["logFile"], "All threads finished and joined\r\n", true);

					//no need to freeaddinfo here because RainWSA2 does that for us

					WSACleanup();
					break;
				} else if (command == "help") {
					std::cout << "\"exit\" to terminate server\r\n";
				} else {
					std::cout << "Command not recognized\r\n";
				}
			}

			std::cout << "EmiliaMailServer has exited. Exiting in 2 seconds...\r\n\r\n";
			Rain::fastOutputFile(config["logFile"], "EmiliaMailServer has exited. Exiting in 2 seconds...\r\n\r\n", true);
			Sleep(2000);

			return 0;
		}
	}
}