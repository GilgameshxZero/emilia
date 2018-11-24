#include "Start.h"

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		int start(int argc, char* argv[]) {
			//parameters
			std::map<std::string, std::string> config;

			std::string configLocFile = "config-loc.ini";
			Rain::concatMap(config, Rain::readParameterFile(configLocFile));
			std::string configFile = config["config-path"] + config["config-file"];
			Rain::concatMap(config, Rain::readParameterFile(configFile));
			std::string authenticationFile = config["config-path"] + config["auth-file"];
			Rain::concatMap(config, Rain::readParameterFile(authenticationFile));

			//debugging & logging
			Rain::createDirRec(config["log-path"]);
			Rain::redirectCerrFile(config["log-path"] + config["log-error"], true);
			HANDLE hFMemLeak = Rain::logMemoryLeaks(config["log-path"] + config["log-memory"]);

			Rain::LogStream logger;
			logger.setFileDst(config["log-path"] + config["log-log"], true);
			logger.setStdHandleSrc(STD_OUTPUT_HANDLE, true);

			//output parameters
			Rain::tsCout("Starting...\r\n", config.size(), " configuration options:\r\n");
			for (std::map<std::string, std::string>::iterator it = config.begin(); it != config.end(); it++)
				Rain::tsCout("\t" + it->first + ": " + it->second + "\r\n");

			Rain::tsCout("\r\nCommand line arguments: " + Rain::tToStr(argc) + "\r\n\r\n");
			for (int a = 0; a < argc; a++) {
				Rain::tsCout(std::string(argv[a]) + "\r\n");
			}
			Rain::tsCout("\r\n");

			//set up command handlers
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
			CommandHandlerParam cmhParam;
			cmhParam.config = &config;
			cmhParam.logger = &logger;

			//check command line, perhaps we are being restarted by helper and need to display a message
			if (argc >= 2) {
				std::string arg1 = argv[1];
				Rain::strTrimWhite(&arg1);
				if (arg1 == "stage-dev-crh-success")
					Rain::tsCout("IMPORTANT: 'stage-dev' CRH completed successfully.\r\n");
				else if (arg1 == "stage-prod-crh-success")
					Rain::tsCout("IMPORTANT: 'stage-prod' CRH completed successfully.\r\n");
				else if (arg1 == "deploy-staging-crh-success") {
					Rain::tsCout("IMPORTANT: 'deploy-staging' CRH completed successfully.\r\n");
					CHProdStart(cmhParam);
				}
			}

			Rain::tsCout("\r\n");

			//command loop
			while (true) {
				static std::string command, tmp;
				Rain::tsCout("Accepting commands...\r\n");
				std::cin >> command;
				Rain::tsCout("Command: " + command + "\r\n");
				std::getline(std::cin, tmp);

				if (command == "exit") {
					break;
				} else {
					auto handler = commandHandlers.find(command);
					if (handler != commandHandlers.end()) {
						if (handler->second(cmhParam) != 0)
							break;
					} else {
						Rain::tsCout("Command not recognized.\r\n");
					}
				}
			}

			logger.setStdHandleSrc(STD_OUTPUT_HANDLE, false);

			Rain::tsCout("The program has terminated. Exiting in 3 seconds...\r\n");
			fflush(stdout);
			Sleep(3000);
			return 0;
		}
	}
}