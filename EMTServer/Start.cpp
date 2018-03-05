#include "Start.h"

namespace Mono3 {
	int start() {
		Rain::redirectCerrFile("NoSync\\errors.txt");
		Rain::logMemoryLeaks("NoSync\\memoryLeaks.txt");

		std::map<std::string, std::string> params;
		Rain::readParameterFile("config.ini", params);

		std::cout << "Configuration parameters: \n"
			<< "\tServer listening port: " << params["serverListenPort"] << "\n"
			<< "\tServer root directory: " << params["serverRootDir"] << "\n";

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
		ListenThreadParam ltParam;
		ltParam.lSocket = &lSocket;
		ltParam.ltLLMutex = new std::mutex;

		ltParam.end = ltParam.beg = NULL;
		ltParam.serverRootDir = params["serverRootDir"];

		CreateThread(NULL, 0, Mono3::listenThread, reinterpret_cast<LPVOID>(&ltParam), 0, NULL);

		std::string command, tmp;
		while (true) {
			std::cout << "Accepting commands...\n";
			std::cin >> command;
			std::getline(std::cin, tmp);

			if (command == "exit") {
				//first, close the listening socket so that all calls to 'accept' terminate
				closesocket(lSocket);

				//shutdown threads, socket sends, and WSACleanup
				//send WM_DESTROY to all threads
				ltParam.ltLLMutex->lock();
				CTLLNode *cur = ltParam.beg;
				while (cur != NULL) {
					//use postmessage here because we want the thread of the window to process the message, allowing destroywindow to be called
					//WM_RAINAVAILABLE + 1 is the end message
					PostMessage(cur->hwnd, WM_RAINAVAILABLE + 1, 0, 0);

					cur = cur->next;
				}
				ltParam.ltLLMutex->unlock();

				//wait on all threads to terminate
				Sleep(1000);//todo

				WSACleanup();
				break;
			}
		}

		//free memory
		delete ltParam.ltLLMutex;

		std::cout << "The server has terminated. Exiting autmatically in 1 second...";
		Sleep(1000);

		return 0;
	}
}