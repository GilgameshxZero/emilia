#include "Start.h"

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		int start(int argc, char* argv[]) {
			//parameters
			std::map<std::string, std::string> config;

			std::string configLocFile = "config-loc.ini";
			Rain::readParameterFile(configLocFile, config);
			std::string configFile = config["config-path"] + config["config-file"];
			Rain::readParameterFile(configFile, config);
			std::string authenticationFile = config["config-path"] + config["auth-file"];
			Rain::readParameterFile(authenticationFile, config);

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

			//check command line, perhaps we are being restarted by helper and need to display a message
			if (argc >= 2) {
				std::string arg1 = argv[1];
				Rain::strTrimWhite(arg1);
				if (arg1 == "staging-crh-success")
					Rain::outLogStd("IMPORTANT: Staging operation delayed helper script (EmiliaUpdateCRHelper) completed successfully.\r\n\r\n");
			}

			//command loop
			static std::map<std::string, CommandMethodHandler> commandHandlers{
				{"help", CHHelp},
				{"stage-dev", CHStageDev},
				{"deploy-staging", CHDeployStaging},
				{"prod-download", CHProdDownload},
				{"stage-prod", CHStageProd},
				{"prod-stop", CHProdStop},
				{"prod-start", CHProdStart},
				{"sync-stop", CHSyncStop},
				{"sync-start", CHSyncStart},
			};
			while (true) {
				static std::string command, tmp;
				Rain::outLogStd("Accepting commands...\r\n");
				std::cin >> command;
				Rain::outLogStd("Command: " + command + "\r\n");
				std::getline(std::cin, tmp);

				if (command == "exit") {
					break;
				} else {
					auto handler = commandHandlers.find(command);
					if (handler != commandHandlers.end()) {
						if (handler->second(config) != 0)
							break;
					} else {
						Rain::outLogStd("Command not recognized\r\n");
					}
				}
			}

			Rain::outLogStd("The program has terminated. Exiting in 3 seconds...\r\n");
			Sleep(3000);
			return 0;
		}
	}
}