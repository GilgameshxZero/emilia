#include "Start.h"

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
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

			//check command line for notifications
			if (argc >= 2) {
				std::string arg1 = argv[1];
				Rain::strTrimWhite(&arg1);
				if (arg1 == "prod-upload-success") {
					Rain::tsCout("IMPORTANT: 'prod-upload' CRH operation completed successfully.\r\n\r\n");
				}
			}

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

			HANDLE hCommandWait[2] = {ccParam.hInputExitEvent,
				GetStdHandle(STD_INPUT_HANDLE)};

			//command loop
			Rain::tsCout("Accepting commands...\r\n");
			while (true) {
				static std::string command, cmdLeftover;
				static DWORD waitRtrn;
				static bool commandComplete = false;

				fflush(stdout);
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
								inputs[a].Event.KeyEvent.bKeyDown == TRUE && 
								inputs[a].Event.KeyEvent.uChar.AsciiChar != '\0') {
								command += inputs[a].Event.KeyEvent.uChar.AsciiChar;
								Rain::tsCout(inputs[a].Event.KeyEvent.uChar.AsciiChar);

								//TODO: pressing enter only inputs \r for some reason, so append \n if that's the case
								if (inputs[a].Event.KeyEvent.uChar.AsciiChar == '\r') {
									command += '\n';
									Rain::tsCout('\n');
								}

								fflush(stdout);
							}
						}
						delete[] inputs;

						//check if there's a newline somewhere
						static size_t newlinePos;
						newlinePos = command.find('\n');
						if (newlinePos != std::string::npos) {
							cmdLeftover = command.substr(newlinePos + 1, command.length());
							command = command.substr(0, newlinePos);
							Rain::strTrimWhite(&command);
							commandComplete = true;
						}
					}
				}

				if (commandComplete) {
					commandComplete = false;
					Rain::tsCout("Command: " + command + "\r\n");

					if (command == "exit") {
						Rain::tsCout("Preparing to exit...\r\n");
						break;
					} else {
						Rain::tsCout("Command not recognized.\r\n");
					}

					command = cmdLeftover;
					Rain::tsCout("Accepting commands...\r\n");
				}
			}

			sm.~ServerManager();
			WSACleanup();
			if (hFMemLeak != NULL)
				CloseHandle(hFMemLeak);

			Rain::tsCout("The program has terminated. Exiting in 3 seconds...\r\n");

			//clean up logger before we exit, or else we will get error
			logger.setStdHandleSrc(STD_OUTPUT_HANDLE, false);

			Sleep(3000);
			return 0;
		}
	}
}