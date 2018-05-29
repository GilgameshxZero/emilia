#include "Start.h"

namespace Monochrome3 {
	namespace EMTServer {
		int start() {
			std::string configFile = "config.ini";

			//parameters
			std::map<std::string, std::string> config;
			Rain::readParameterFile(configFile, config);

			//debugging purposes
			Rain::redirectCerrFile(config["errorLog"], true);
			HANDLE hFMemLeak = Rain::logMemoryLeaks(config["memoryLeakLog"]);

			Rain::outLogStdTrunc("Starting server...\r\nRead " + Rain::tToStr(config.size()) + " configuration options:\r\n", 0, config["serverLog"]);

			//output parameters
			for (std::map<std::string, std::string>::iterator it = config.begin(); it != config.end(); it++)
				Rain::outLogStdTrunc("\t" + it->first + ": " + it->second + "\r\n");

			//winsock setup
			WSADATA wsaData;
			struct addrinfo *sAddr;
			SOCKET lSocket;
			if (Rain::quickServerInit(wsaData, config["serverListenPort"], &sAddr, lSocket)) {
				Rain::reportError(GetLastError(), "error in quickServerInit");
				return -1;
			}
			Rain::outLogStdTrunc("EMTServer initialized\r\n");
			if (Rain::listenServSocket(lSocket)) {
				Rain::reportError(GetLastError(), "error in listenServSocket");
				return -1;
			}
			Rain::outLogStdTrunc("Listening...\r\n");

			//continuously accept clients on listening socket using ListenThread
			ConnectionCallerParam ccParam;
			ccParam.config = &config;
			HANDLE hListenThread = Rain::createListenThread(lSocket, reinterpret_cast<void *>(&ccParam), Rain::strToT<std::size_t>(config["recvBufferLength"]), onConnectionProcessMessage, onConnectionInit, onConnectionExit);

			//command loop
			while (true) {
				static std::string command, tmp;
				Rain::outLogStdTrunc("Accepting commands...\r\n");
				std::cin >> command;
				std::getline(std::cin, tmp);

				if (command == "exit") {
					//first, close the listening socket so that all calls to 'accept' terminate
					closesocket(lSocket);
					WaitForSingleObject(hListenThread, 0);
					CloseHandle(hListenThread);
					Rain::outLogStdTrunc("Listening shutdown and ListenThread joined\r\n");

					//no need to freeaddinfo here because RainWSA2 does that for us
					WSACleanup();
					break;
				} else {
					Rain::outLogStdTrunc("Command not recognized\r\n");
				}
			}

			if (hFMemLeak != NULL)
				CloseHandle(hFMemLeak);

			Rain::outLogStdTrunc("The server has terminated. Exiting automatically in 1 second...\r\n");
			Sleep(1000);

			return 0;
		}
	}
}