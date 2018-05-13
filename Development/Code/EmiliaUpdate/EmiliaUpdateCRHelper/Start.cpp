#include "Start.h"

namespace Monochrome3 {
	namespace EmiliaUpdateCRHelper {
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

			//execute a copy command, then a run command, based on command line
			//if command line is incompatible, error and exit
			Rain::outLogStd("Command line arguments: " + Rain::tToStr(argc) + "\r\n\r\n");
			for (int a = 0; a < argc; a++) {
				Rain::outLogStd(std::string(argv[a]) + "\r\n");
			}

			if (argc != 3) {//need a copy source and destination
				Rain::outLogStd("\r\nThere are not the correct number of command line arguments. Exiting in 3 seconds...\r\n");
				Sleep(3000);
				return 0;
			}

			std::string source = argv[1],
				dest = argv[2];

			//wait for destination handle to be free, and then copy source to destination
			//no API, so use progressive delays
			Rain::outLogStd("\r\nWaiting for destination to be available for writing...\r\n");
			HANDLE hDest;
			int delay = 10;
			while ((hDest = CreateFile(dest.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE) {
				if (GetLastError() == ERROR_SHARING_VIOLATION) {
					Sleep(delay);
					if (delay < 5000) //max delay around 7.5 seconds
						delay = delay * 3 / 2;
				} else {
					Rain::outLogStd("Unknown error occured. Exiting in 3 seconds...\r\n");
					Sleep(3000);
					return 0;
				}
			}

			//copy file
			Rain::outLogStd("Copying source to destination...\r\n");
			CloseHandle(hDest);
			CopyFile(source.c_str(), dest.c_str(), FALSE);

			//run destination file
			Rain::outLogStd("Starting destination as executable...\r\n");
			ShellExecute(NULL, "open", dest.c_str(), "staging-crh-success", Rain::pathToDir(dest).c_str(), SW_SHOWDEFAULT);

			Rain::outLogStd("The program has terminated. Exiting in 3 seconds...\r\n");
			Sleep(3000);
			return 0;
		}
	}
}