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

			//network setup for receiving mail
			ConnectionCallerParam ccParam;
			ccParam.config = &config;
			ccParam.logger = &logger;
			ccParam.connectedClients = 0;

			Rain::ServerManager sm;
			sm.setEventHandlers(onConnect, onMessage, onDisconnect, &ccParam);
			sm.setRecvBufLen(Rain::strToT<std::size_t>(config["transfer-buflen"]));
			if (!sm.setServerListen(25, 25)) {
				Rain::tsCout("Server listening on port ", sm.getListeningPort(), ".\r\n");
			} else {
				Rain::tsCout("Fatal error: could not setup server listening.\r\n");
				fflush(stdout);
				DWORD error = GetLastError();
				Rain::reportError(error, "Fatal error: could not setup server listening.");
				WSACleanup();
				if (hFMemLeak != NULL)
					CloseHandle(hFMemLeak);
				return error;
			}

			//process commands
			static std::map<std::string, CommandMethodHandler> commandHandlers{
				{"exit", CHExit},
				{"help", CHHelp},
			};
			CommandHandlerParam cmhParam;
			cmhParam.config = &config;
			cmhParam.logger = &logger;
			while (true) {
				static std::string command, tmp;
				Rain::tsCout("Accepting commands...\r\n");
				std::cin >> command;
				Rain::tsCout("Command: " + command + "\r\n");
				std::getline(std::cin, tmp);

				auto handler = commandHandlers.find(command);
				if (handler != commandHandlers.end()) {
					if (handler->second(cmhParam) != 0)
						break;
				} else {
					Rain::tsCout("Command not recognized.\r\n");
				}
			}

			Rain::tsCout("The program has terminated. Exiting in 3 seconds...\r\n");
			fflush(stdout);
			Sleep(3000);
			return 0;
		}
	}
}