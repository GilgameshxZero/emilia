#include "CommandHandlers.h"

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		int CHHelp(std::map<std::string, std::string> &config) {
			//see readme for what each of these do
			Rain::outLogStd("Available commands: exit, help, stage-dev, deploy-staging, prod-download, stage-prod, prod-stop, prod-start, sync-stop, sync-start\r\n");
			return 0;
		}
		int CHStageDev(std::map<std::string, std::string> &config) {
			//verify directory structures
			Rain::outLogStd("Checking directory structure...\r\n");
			std::string rootDir = "..\\..\\..\\..\\";
			std::vector<std::string> rootDirs;
			Rain::getDirectories(rootDir, rootDirs);
			if (std::find(rootDirs.begin(), rootDirs.end(), "Development") == rootDirs.end()) {
				Rain::outLogStd("/Development not present\r\n");
				return 0;
			}
			if (std::find(rootDirs.begin(), rootDirs.end(), "Staging") == rootDirs.end()) {
				Rain::outLogStd("/Staging not present\r\n");
				return 0;
			}

			std::string devDir = rootDir + "Development\\",
				stagingDir = rootDir + "Staging\\";
			std::vector<std::string> devDirs, stagingDirs;
			Rain::getDirectories(devDir, devDirs);
			Rain::getDirectories(stagingDir, stagingDirs);
			if (std::find(devDirs.begin(), devDirs.end(), "Code") == devDirs.end()) {
				Rain::outLogStd("/Development/Code not present\r\n");
				return 0;
			}
			if (std::find(stagingDirs.begin(), stagingDirs.end(), "Code") == stagingDirs.end()) {
				Rain::outLogStd("/Staging/Code not present\r\n");
				return 0;
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
					crhCmdLine = "\"" + delayPath + "\" \"" +
					thisPath + "\"" +
					"staging-crh-success";
				crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
				ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);

				Rain::outLogStd("\r\nThe current executable needs to be replaced. It will be restarted when the operation is complete.\r\n");
				return 1;
			}

			Rain::outLogStd("\r\nStaging complete.\r\n");
			return 0;
		}
		int CHDeployStaging(std::map<std::string, std::string> &config) {
			//request over the network
			Rain::NetworkClientManager ncm;
			ncm.setClientTarget(config["server-ip"], Rain::strToT<DWORD>(config["server-port-low"]), Rain::strToT<DWORD>(config["server-port-high"]));
			ncm.blockForConnect(0);
			Rain::outLogStd("Connected to server.\r\n");

			Rain::sendBlockMessage(ncm, "authenticate " + config["client-auth-pass"]);
			Rain::sendBlockMessage(ncm, "prod-stop");
			Rain::sendBlockMessage(ncm, "prod-download");

			//upload files

			Sleep(100000);
			return 0;
		}
		int CHProdDownload(std::map<std::string, std::string> &config) {
			return 0;
		}
		int CHStageProd(std::map<std::string, std::string> &config) {
			return 0;
		}
		int CHProdStop(std::map<std::string, std::string> &config) {
			return 0;
		}
		int CHProdStart(std::map<std::string, std::string> &config) {
			return 0;
		}
		int CHSyncStop(std::map<std::string, std::string> &config) {
			return 0;
		}
		int CHSyncStart(std::map<std::string, std::string> &config) {
			return 0;
		}
	}
}