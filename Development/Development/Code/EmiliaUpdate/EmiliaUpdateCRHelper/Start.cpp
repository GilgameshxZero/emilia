#include "Start.h"

namespace Monochrome3 {
	namespace EmiliaUpdateCRHelper {
		int start(int argc, char* argv[]) {
			//parameters
			std::map<std::string, std::string> config;

			std::string configLocFile = "config-loc.ini";
			Rain::concatMap(config, Rain::readParameterFile(configLocFile));
			std::string configFile = config["config-path"] + config["config-file"];
			Rain::concatMap(config, Rain::readParameterFile(configFile));

			//debugging & logging
			Rain::createDirRec(config["aux-path"]);
			Rain::redirectCerrFile(config["aux-path"] + config["aux-error"], true);
			HANDLE hFMemLeak = Rain::logMemoryLeaks(config["aux-path"] + config["aux-memory"]);

			Rain::LogStream logger;
			logger.setFileDst(config["aux-path"] + config["aux-log"], true);
			logger.setStdHandleSrc(STD_OUTPUT_HANDLE, true);

			//output parameters
			Rain::tsCout("Starting...\r\n" + Rain::tToStr(config.size()) + " configuration options:\r\n", config["aux-path"] + config["aux-log"]);
			for (std::map<std::string, std::string>::iterator it = config.begin(); it != config.end(); it++)
				Rain::tsCout("\t" + it->first + ": " + it->second + "\r\n");

			//execute a copy command, then a run command, based on command line
			//if command line is incompatible, error and exit
			Rain::tsCout("Command line arguments: " + Rain::tToStr(argc) + "\r\n\r\n");
			for (int a = 0; a < argc; a++) {
				Rain::tsCout(std::string(argv[a]) + "\r\n");
			}

			if (argc < 3) {//need a copy source and destination
				Rain::tsCout("\r\nThere are not the correct number of command line arguments. Exiting in 3 seconds...\r\n");
				logger.setStdHandleSrc(STD_OUTPUT_HANDLE, false);
				Sleep(3000);
				return 0;
			}

			std::string source = argv[1],
				dest = argv[2];

			//wait for destination handle to be free, and then copy source to destination
			//no API, so use progressive delays
			Rain::tsCout("\r\nWaiting for destination to be available for writing...\r\n");
			fflush(stdout);
			HANDLE hDest;
			int delay = 10;
			while ((hDest = CreateFile(dest.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE) {
				if (GetLastError() == ERROR_SHARING_VIOLATION) {
					Sleep(delay);
					if (delay < 5000) //max delay around 7.5 seconds
						delay = delay * 3 / 2;
				} else {
					Rain::tsCout("Unknown error occured. Exiting in 3 seconds...\r\n");
					logger.setStdHandleSrc(STD_OUTPUT_HANDLE, false);
					Sleep(3000);
					return 0;
				}
			}

			//copy file
			Rain::tsCout("Copying source to destination...\r\n");
			CloseHandle(hDest);
			CopyFile(source.c_str(), dest.c_str(), FALSE);

			//run destination file with any additional arguments passed to the cmdLine
			Rain::tsCout("Starting destination as executable...\r\n");
			std::string cmdLine = "";
			for (int a = 3; a < argc; a++)
				cmdLine += std::string(argv[a]) + " ";
			if (cmdLine.length() > 1)
				cmdLine.pop_back(); //remove the trailing space
			ShellExecute(NULL, "open", dest.c_str(), cmdLine.c_str(), Rain::getPathDir(dest).c_str(), SW_SHOWDEFAULT);

			Rain::tsCout("The program has terminated. Exiting in 3 seconds...\r\n");
			fflush(stdout);

			//clean up logger before we exit, or else we will get error
			logger.setStdHandleSrc(STD_OUTPUT_HANDLE, false);

			Sleep(3000);
			return 0;
		}
	}
}