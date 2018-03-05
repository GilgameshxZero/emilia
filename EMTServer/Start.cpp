#include "Start.h"

namespace Mono3 {
	int start() {
		//parameters
		std::map<std::string, std::string> params;
		Rain::readParameterFile("config.ini", params);

		std::cout << "Configuration parameters: \n"
			<< "\tServer listening port: " << params["serverListenPort"] << "\n"
			<< "\tServer root directory: " << params["serverRootDir"] << "\n"
			<< "\tServer auxillary files: " << params["serverAuxiliary"] << "\n";

		//debugging purposes
		Rain::redirectCerrFile(params["serverAuxiliary"] + "errors.txt");
		Rain::logMemoryLeaks(params["serverAuxiliary"] + "memoryLeaks.txt");

		//winsock setup
		WSADATA wsaData;
		struct addrinfo *sAddr;
		SOCKET lSocket;
		int error = Rain::quickServerInit(wsaData, params["serverListenPort"], &sAddr, lSocket);
		if (error) {
			Rain::reportError(error, "error in quickServerInit");
			return error;
		}
		std::cout << "Server initialized\n";
		error = Rain::listenServSocket(lSocket);
		if (error) {
			Rain::reportError(error, "error in listenServSocket");
			return error;
		}
		std::cout << "Listening setup\n";

		//main thread responsible for capturing cin and commands
		//spawn listening threads to deal with socket connections
		//arrange threads in a linked list, with dummy beginning and end nodes
		//ltParam needs to be dynamically allocated, because it will be freed by listenThread
		//however, the mutex should be allocated outside ltParam, because we will need to use it again even after the threads have exited
		std::mutex *ltLLMutex = new std::mutex;

		ListenThreadParam *ltParam = new ListenThreadParam();
		ltParam->lSocket = &lSocket;
		ltParam->ltLLMutex = ltLLMutex;
		ltParam->serverRootDir = &params["serverRootDir"];
		ltParam->serverAux = &params["serverAuxiliary"];

		//create dummy head and tail nodes for the ListenThreadParam linked list, and put the new LTP in between
		ListenThreadParam LTPLLHead, LTPLLTail;
		LTPLLHead.prevLTP = NULL;
		LTPLLHead.nextLTP = ltParam;
		ltParam->prevLTP = &LTPLLHead;
		ltParam->nextLTP = &LTPLLTail;
		LTPLLTail.prevLTP = ltParam;
		LTPLLTail.nextLTP = NULL;

		CreateThread(NULL, 0, Mono3::listenThread, reinterpret_cast<LPVOID>(ltParam), 0, NULL);

		//at this point, we should assume that ltParam may be deleted already

		//command loop
		while (true) {
			static std::string command, tmp;
			std::cout << "Accepting commands...\n";
			std::cin >> command;
			std::getline(std::cin, tmp);

			if (command == "exit") {
				//first, close the listening socket so that all calls to 'accept' terminate
				closesocket(lSocket);
				std::cout << "Socket listen closed\n";

				//shutdown threads by walking through the linked list and sending the end message
				ltLLMutex->lock();
				ListenThreadParam *curLTP = LTPLLHead.nextLTP;
				while (curLTP != &LTPLLTail) {
					//use postmessage here because we want the thread of the window to process the message, allowing destroywindow to be called
					//WM_RAINAVAILABLE + 1 is the end message
					PostMessage(curLTP->rainWnd.hwnd, WM_RAINAVAILABLE + 1, 0, 0);
					curLTP = curLTP->nextLTP;
				}
				ltLLMutex->unlock();

				//wait on all threads to terminate
				Sleep(1000);//todo

				//no need to freeaddinfo here because RainWSA2 does that for us

				WSACleanup();
				std::cout << "WSA cleaned up\n";

				break;
			} else if (command == "help") {
				std::cout << "\"exit\" to terminate server\n";
			} else {
				std::cout << "Command not recognized\n";
			}
		}

		//free memory
		delete ltLLMutex;

		std::cout << "The server has terminated. Exiting autmatically in 1 second...";
		Sleep(1000);

		return 0;
	}
}