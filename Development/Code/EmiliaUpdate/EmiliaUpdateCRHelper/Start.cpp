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
			Rain::outLogStdTrunc("Starting client...\r\n" + Rain::tToStr(config.size()) + " configuration options:\r\n", 0, config["aux-path"] + config["aux-log"]);
			for (std::map<std::string, std::string>::iterator it = config.begin(); it != config.end(); it++)
				Rain::outLogStdTrunc("\t" + it->first + ": " + it->second + "\r\n");

			//command loop
			while (true) {
				static std::string command, tmp;
				Rain::outLogStdTrunc("Accepting commands...\r\n");
				std::cin >> command;
				Rain::outLogStdTrunc("Command: " + command + "\r\n");
				std::getline(std::cin, tmp);

				if (command == "exit") {
					break;
				} else if (command == "help") {
					//see readme for what each of these do
					Rain::outLogStdTrunc("Available commands: exit, help, stage, deploy, download, prod-stop, prod-start, sync-stop, sync-start\r\n");
				} else if (command == "stage") {
					//verify directory structures
					Rain::outLogStdTrunc("Checking directory structure...\r\n");
					std::string rootDir = "..\\..\\..\\..\\";
					std::vector<std::string> rootDirs;
					Rain::getDirectories(rootDir, rootDirs);
					if (std::find(rootDirs.begin(), rootDirs.end(), "Development") == rootDirs.end()) {
						Rain::outLogStdTrunc("/Development not present\r\n");
						continue;
					}
					if (std::find(rootDirs.begin(), rootDirs.end(), "Staging") == rootDirs.end()) {
						Rain::outLogStdTrunc("/Staging not present\r\n");
						continue;
					}

					std::string devDir = rootDir + "Development\\",
						stagingDir = rootDir + "Staging\\";
					std::vector<std::string> devDirs, stagingDirs;
					Rain::getDirectories(devDir, devDirs);
					Rain::getDirectories(stagingDir, stagingDirs);
					if (std::find(devDirs.begin(), devDirs.end(), "Code") == devDirs.end()) {
						Rain::outLogStdTrunc("/Development/Code not present\r\n");
						continue;
					}
					if (std::find(stagingDirs.begin(), stagingDirs.end(), "Code") == stagingDirs.end()) {
						Rain::outLogStdTrunc("/Staging/Code not present\r\n");
						continue;
					}

					Rain::outLogStdTrunc("Removing files in /Staging/Code...\r\n");
					std::string devCodeDir = devDir + "Code\\",
						stagingCodeDir = stagingDir + "Code\\";
					Rain::recursiveRmDir(stagingCodeDir);

					//stage relevant files
					Rain::outLogStdTrunc("Copying relevant files from /Development/Code to /Staging/Code...\r\n\r\n");
					std::vector<std::string> stagingFiles;
					Rain::readMultilineFile(config["config-path"] + config["staging-files"], stagingFiles);

					//if we are replacing the current executable, then delay the replace until after the program exits
					std::string thisPath = Rain::getFullPathStr(Rain::getExecutablePath()),
						delayPath = "";

					for (std::string file : stagingFiles) {
						if (!Rain::fileExists(devCodeDir + file)) {
							Rain::outLogStdTrunc("Doesn't exist:\t");
						} else {
							if (thisPath == Rain::getFullPathStr(stagingCodeDir + file)) {
								delayPath = thisPath;
								Rain::outLogStdTrunc("Delayed:\t");
							} else {
								Rain::createDirRec(Rain::pathToDir(stagingCodeDir + file));
								if (!CopyFile((devCodeDir + file).c_str(), (stagingCodeDir + file).c_str(), FALSE)) {
									Rain::outLogStdTrunc("Error:\t\t");
								} else {
									Rain::outLogStdTrunc("Copied:\t\t");
								}
							}
						}
						Rain::outLogStdTrunc(file + "\r\n");
					}

					//if we need to replace the current script, run CRHelper, which will restart the current script when complete
					if (delayPath != "") {
						Rain::outLogStdTrunc("\r\nThe current executable needs to be replaced. It will be restarted when the operation is complete. Exiting in 3 seconds...\r\n");
						Sleep(3000);
						return 0;
					}

					Rain::outLogStdTrunc("\r\nStaging complete.\r\n");
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

			Rain::outLogStdTrunc("The client has terminated. Exiting in 3 seconds...\r\n");
			Sleep(3000);
			return 0;
		}
	}
}