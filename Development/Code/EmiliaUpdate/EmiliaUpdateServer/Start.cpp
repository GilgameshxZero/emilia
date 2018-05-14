#include "Start.h"

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		int start(int argc, char* argv[]) {
			//parameters
			std::map<std::string, std::string> config;

			std::string configLocFile = "config-loc.ini";
			Rain::readParameterFile(configLocFile, config);
			std::string configFile = config["config-path"] + config["config-file"];
			Rain::readParameterFile(configFile, config);

			//debugging
			Rain::redirectCerrFile(config["aux-path"] + config["aux-error"], true);
			HANDLE hFMemLeak = Rain::logMemoryLeaks(config["aux-path"] + config["aux-memory"]);

			//output parameters
			Rain::outLogStd("Starting...\r\n" + Rain::tToStr(config.size()) + " configuration options:\r\n", config["aux-path"] + config["aux-log"]);
			for (std::map<std::string, std::string>::iterator it = config.begin(); it != config.end(); it++)
				Rain::outLogStd("\t" + it->first + ": " + it->second + "\r\n");

			Rain::outLogStd("\r\nCommand line arguments: " + Rain::tToStr(argc) + "\r\n\r\n");
			for (int a = 0; a < argc; a++) {
				Rain::outLogStd(std::string(argv[a]) + "\r\n");
			}
			Rain::outLogStd("\r\n");

			//check command line for notifications
			if (argc >= 2) {
				//do nothing
			}

			//set up network stuff to continuously accept sockets
			WSADATA wsaData;
			struct addrinfo *sAddr;
			SOCKET lSocket;
			if (Rain::quickServerInit(wsaData, config["first-port"], &sAddr, lSocket)) {
				Rain::reportError(GetLastError(), "error in quickServerInit");
				return -1;
			}
			if (Rain::listenServSocket(lSocket)) {
				Rain::reportError(GetLastError(), "error in listenServSocket");
				return -1;
			}
			Rain::outLogStd("Socket listening...\r\n");

			ConnectionCallerParam ccParam;
			ccParam.config = &config;
			HANDLE hListenThread = Rain::createListenThread(lSocket, reinterpret_cast<void *>(&ccParam), Rain::strToT<std::size_t>(config["recv-buflen"]), onConnectionProcessMessage, onConnectionInit, onConnectionExit);

			//command loop
			while (true) {
				static std::string command, tmp;
				Rain::outLogStd("Accepting commands...\r\n");
				std::cin >> command;
				Rain::outLogStd("Command: " + command + "\r\n");
				std::getline(std::cin, tmp);

				if (command == "exit") {
					//first, close the listening socket so that all calls to 'accept' terminate
					closesocket(lSocket);
					WaitForSingleObject(hListenThread, 0);
					CloseHandle(hListenThread);
					Rain::outLogStd("Listening shutdown and ListenThread joined.\r\n");

					//no need to freeaddinfo here because RainWSA2 does that for us
					WSACleanup();
				} else {
					Rain::outLogStd("Command not recognized\r\n");
				}
			}

			if (hFMemLeak != NULL)
				CloseHandle(hFMemLeak);

			Rain::outLogStd("The program has terminated. Exiting in 3 seconds...\r\n");
			Sleep(3000);
			return 0;
		}
	}
}