#include "Start.h"

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		int start() {
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
			Rain::outLogStdTrunc("Starting server...\r\n" + Rain::tToStr(config.size()) + " configuration options:\r\n", 0, config["aux-path"] + config["aux-log"]);
			for (std::map<std::string, std::string>::iterator it = config.begin(); it != config.end(); it++)
				Rain::outLogStdTrunc("\t" + it->first + ": " + it->second + "\r\n");

			//command loop
			while (true) {
				static std::string command, tmp;
				Rain::outLogStdTrunc("Accepting commands...\r\n");
				std::cin >> command;
				Rain::outLogStdTrunc(command + "\r\n");
				std::getline(std::cin, tmp);

				if (command == "exit") {
					break;
				} else if (command == "help") {
					Rain::outLogStdTrunc("Available commands: exit, help, stage, deploy, download, prod-stop, prod-start, sync-stop, sync-start\r\n");
				} else if (command == "stage") {
				} else if (command == "deploy") {
				} else if (command == "download") {
				} else if (command == "prod-stop") {
				} else if (command == "prod-start") {
				} else if (command == "sync-stop") {
				} else if (command == "sync-start") {
				} else {
					Rain::outLogStdTrunc("Command not recognized\r\n");
				}
			}

			Rain::outLogStdTrunc("The server has terminated. Exiting in 3 seconds...\r\n");
			Sleep(3000);

			return 0;
		}
	}
}