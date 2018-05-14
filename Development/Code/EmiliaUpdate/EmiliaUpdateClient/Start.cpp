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
				Rain::strTrim(arg1);
				if (arg1 == "staging-crh-success")
					Rain::outLogStd("IMPORTANT: Staging operation delayed helper script (EmiliaUpdateCRHelper) completed successfully.\r\n\r\n");
			}

			//command loop
			while (true) {
				static std::string command, tmp;
				Rain::outLogStd("Accepting commands...\r\n");
				std::cin >> command;
				Rain::outLogStd("Command: " + command + "\r\n");
				std::getline(std::cin, tmp);

				if (command == "exit") {
					break;
				} else if (command == "help") {
					//see readme for what each of these do
					Rain::outLogStd("Available commands: exit, help, stage, deploy, download, prod-stop, prod-start, sync-stop, sync-start\r\n");
				} else if (command == "stage-dev") {
					//verify directory structures
					Rain::outLogStd("Checking directory structure...\r\n");
					std::string rootDir = "..\\..\\..\\..\\";
					std::vector<std::string> rootDirs;
					Rain::getDirectories(rootDir, rootDirs);
					if (std::find(rootDirs.begin(), rootDirs.end(), "Development") == rootDirs.end()) {
						Rain::outLogStd("/Development not present\r\n");
						continue;
					}
					if (std::find(rootDirs.begin(), rootDirs.end(), "Staging") == rootDirs.end()) {
						Rain::outLogStd("/Staging not present\r\n");
						continue;
					}

					std::string devDir = rootDir + "Development\\",
						stagingDir = rootDir + "Staging\\";
					std::vector<std::string> devDirs, stagingDirs;
					Rain::getDirectories(devDir, devDirs);
					Rain::getDirectories(stagingDir, stagingDirs);
					if (std::find(devDirs.begin(), devDirs.end(), "Code") == devDirs.end()) {
						Rain::outLogStd("/Development/Code not present\r\n");
						continue;
					}
					if (std::find(stagingDirs.begin(), stagingDirs.end(), "Code") == stagingDirs.end()) {
						Rain::outLogStd("/Staging/Code not present\r\n");
						continue;
					}

					Rain::outLogStd("Removing files in /Staging/Code...\r\n");
					std::string devCodeDir = devDir + "Code\\",
						stagingCodeDir = stagingDir + "Code\\";
					Rain::recursiveRmDir(stagingCodeDir);

					//stage relevant files
					Rain::outLogStd("Copying relevant files from /Development/Code to /Staging/Code...\r\n\r\n");
					std::vector<std::string> stagingFiles;
					Rain::readMultilineFile(config["config-path"] + config["staging-files"], stagingFiles);

					//if we are replacing the current executable, then delay the replace until after the program exits
					std::string thisPath = Rain::getFullPathStr(Rain::getExecutablePath()),
						delayPath = "";

					for (std::string file : stagingFiles) {
						if (!Rain::fileExists(devCodeDir + file)) {
							Rain::outLogStd("Doesn't exist:\t");
						} else {
							if (thisPath == Rain::getFullPathStr(stagingCodeDir + file)) {
								delayPath = Rain::getFullPathStr(devCodeDir + file);
								Rain::outLogStd("Delayed:\t");
							} else {
								Rain::createDirRec(Rain::pathToDir(stagingCodeDir + file));
								if (!CopyFile((devCodeDir + file).c_str(), (stagingCodeDir + file).c_str(), FALSE)) {
									Rain::outLogStd("Error:\t\t");
								} else {
									Rain::outLogStd("Copied:\t\t");
								}
							}
						}
						Rain::outLogStd(file + "\r\n");
					}

					//if we need to replace the current script, run CRHelper, which will restart the current script when complete
					if (delayPath != "") {
						std::string crhelperAbspath = Rain::getFullPathStr(config["crhelper"]),
							crhWorkingDir = Rain::pathToDir(crhelperAbspath),
							crhCmdLine = "\"" + delayPath + "\" \"" + thisPath + "\"";
						crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
						ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);

						Rain::outLogStd("\r\nThe current executable needs to be replaced. It will be restarted when the operation is complete. Exiting in 3 seconds...\r\n");
						Sleep(3000);

						return 0;
					}

					Rain::outLogStd("\r\nStaging complete.\r\n");
				} else if (command == "deploy-staging") {
				} else if (command == "stage-prod") {
				} else if (command == "download") {
				} else if (command == "prod-stop") {
				} else if (command == "prod-start") {
				} else if (command == "sync-stop") {
				} else if (command == "sync-start") {
				} else {
					Rain::outLogStd("Command not recognized\r\n");
				}
			}

			Rain::outLogStd("The program has terminated. Exiting in 3 seconds...\r\n");
			Sleep(3000);
			return 0;
		}
	}
}