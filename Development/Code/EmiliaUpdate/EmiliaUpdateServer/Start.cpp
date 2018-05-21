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
			std::string authenticationFile = config["config-path"] + config["auth-file"];
			Rain::readParameterFile(authenticationFile, config);

			std::vector<std::string> prodServers;
			Rain::readMultilineFile(config["config-path"] + config["prod-servers"], prodServers);

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
				if (argv[1] == "prod-upload-success") {
					Rain::outLogStd("IMPORTANT: 'prod-upload' main and CRH operation completed successfully.\r\n\r\n");
				}
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
			ccParam.hInputExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			ccParam.serverStatus.resize(prodServers.size());
			ccParam.clientConnected = false;
			for (int a = 0; a < prodServers.size(); a++) {
				ccParam.serverStatus[a].path = Rain::getFullPathStr(prodServers[a]);
				ccParam.serverStatus[a].status = "stopped";
			}

			HANDLE hListenThread = Rain::createListenThread(lSocket, reinterpret_cast<void *>(&ccParam), Rain::strToT<std::size_t>(config["recv-buflen"]), onConnectionProcessMessage, onConnectionInit, onConnectionExit);

			HANDLE hCommandWait[2] = {ccParam.hInputExitEvent,
				GetStdHandle(STD_INPUT_HANDLE)};

			//command loop
			Rain::outLogStd("Accepting commands...\r\n");
			while (true) {
				static std::string command, cmdLeftover;
				static DWORD waitRtrn;
				static bool commandComplete = false;

				waitRtrn = WaitForMultipleObjects(2, hCommandWait, FALSE, INFINITE);
				if (waitRtrn == WAIT_OBJECT_0) {
					command = "exit";
					cmdLeftover = "";
					commandComplete = true;
				} else if (waitRtrn == WAIT_OBJECT_0 + 1) {
					//get user input from low-level console functions
					static DWORD numberOfEvents;
					GetNumberOfConsoleInputEvents(hCommandWait[1], &numberOfEvents);

					if (numberOfEvents > 0) {
						static INPUT_RECORD *inputs;
						static DWORD eventsRead;
						inputs = new INPUT_RECORD[numberOfEvents];
						ReadConsoleInput(hCommandWait[1], inputs, numberOfEvents, &eventsRead);
						for (int a = 0; a < static_cast<int>(numberOfEvents); a++) {
							if (inputs[a].EventType == KEY_EVENT && 
								inputs[a].Event.KeyEvent.bKeyDown == TRUE) {
								command += inputs[a].Event.KeyEvent.uChar.AsciiChar;
								Rain::rainCoutF(inputs[a].Event.KeyEvent.uChar.AsciiChar);

								//TODO: pressing enter only inputs \r for some reason, so append \n if that's the case
								if (inputs[a].Event.KeyEvent.uChar.AsciiChar == '\r') {
									command += '\n';
									Rain::rainCoutF('\n');
								}
							}
						}
						delete[] inputs;

						//check if there's a newline somewhere
						static size_t newlinePos;
						newlinePos = command.find('\n');
						if (newlinePos != std::string::npos) {
							cmdLeftover = command.substr(newlinePos + 1, command.length());
							command = command.substr(0, newlinePos);
							Rain::strTrim(command);
							commandComplete = true;
						}
					}
				}

				if (commandComplete) {
					commandComplete = false;
					Rain::outLogStd("Command: " + command + "\r\n");

					if (command == "exit") {
						//first, close the listening socket so that all calls to 'accept' terminate
						closesocket(lSocket);
						WaitForSingleObject(hListenThread, 0);
						CloseHandle(hListenThread);
						Rain::outLogStd("Listening shutdown and ListenThread joined.\r\n");

						//no need to freeaddinfo here because RainWSA2 does that for us
						WSACleanup();
						break;
					} else {
						Rain::outLogStd("Command not recognized.\r\n");
					}

					command = cmdLeftover;
					Rain::outLogStd("Accepting commands...\r\n");
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