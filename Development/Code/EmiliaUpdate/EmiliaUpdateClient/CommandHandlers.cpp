#include "CommandHandlers.h"

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		int CHHelp(CommandHandlerParam &cmhParam) {
			//see readme for what each of these do
			Rain::tsCout("Available commands: exit, help, stage-dev, deploy-staging, prod-download, stage-prod, prod-stop, prod-start, sync-stop, sync-start\r\n");
			return 0;
		}
		int CHStageDev(CommandHandlerParam &cmhParam) {
			//verify directory structures
			Rain::tsCout("Checking directory structure...\r\n");
			std::string rootDir = "..\\..\\..\\..\\";
			std::vector<std::string> rootDirs;
			rootDirs = Rain::getDirs(rootDir);
			if (std::find(rootDirs.begin(), rootDirs.end(), "Development") == rootDirs.end()) {
				Rain::tsCout("/Development not present\r\n");
				return 0;
			}
			if (std::find(rootDirs.begin(), rootDirs.end(), "Staging") == rootDirs.end()) {
				Rain::tsCout("/Staging not present\r\n");
				return 0;
			}

			std::string devDir = rootDir + "Development\\",
				stagingDir = rootDir + "Staging\\";
			std::vector<std::string> devDirs, stagingDirs;
			devDirs = Rain::getDirs(devDir);
			stagingDirs = Rain::getDirs(stagingDir);
			if (std::find(devDirs.begin(), devDirs.end(), "Code") == devDirs.end()) {
				Rain::tsCout("/Development/Code not present\r\n");
				return 0;
			}
			if (std::find(stagingDirs.begin(), stagingDirs.end(), "Code") == stagingDirs.end()) {
				Rain::tsCout("/Staging/Code not present\r\n");
				return 0;
			}

			Rain::tsCout("Removing files in /Staging/Code...\r\n");
			std::string devCodeDir = devDir + "Code\\",
				stagingCodeDir = stagingDir + "Code\\";
			Rain::recursiveRmDir(stagingCodeDir);

			//stage relevant files
			Rain::tsCout("Copying relevant files from /Development/Code to /Staging/Code...\r\n\r\n");
			std::vector<std::string> stagingFiles;
			stagingFiles = Rain::readMultilineFile((*cmhParam.config)["config-path"] + (*cmhParam.config)["staging-files"]);

			//if we are replacing the current executable, then delay the replace until after the program exits
			std::string thisPath = Rain::pathToAbsolute(Rain::getExePath()),
				delayPath = "";

			for (std::string file : stagingFiles) {
				if (!Rain::fileExists(devCodeDir + file)) {
					Rain::tsCout("Doesn't exist:\t");
				} else {
					if (thisPath == Rain::pathToAbsolute(stagingCodeDir + file)) {
						delayPath = Rain::pathToAbsolute(devCodeDir + file);
						Rain::tsCout("Delayed:\t");
					} else {
						Rain::createDirRec(Rain::getPathDir(stagingCodeDir + file));
						if (!CopyFile((devCodeDir + file).c_str(), (stagingCodeDir + file).c_str(), FALSE)) {
							Rain::tsCout("Error:\t\t");
						} else {
							Rain::tsCout("Copied:\t\t");
						}
					}
				}
				Rain::tsCout(file + "\r\n");
			}

			//if we need to replace the current script, run CRHelper, which will restart the current script when complete
			if (delayPath != "") {
				std::string crhelperAbspath = Rain::pathToAbsolute((*cmhParam.config)["crhelper"]),
					crhWorkingDir = Rain::getPathDir(crhelperAbspath),
					crhCmdLine = "\"" + delayPath + "\" \"" +
					thisPath + "\"" +
					"staging-crh-success";
				crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
				ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);

				Rain::tsCout("\r\nThe current executable needs to be replaced. It will be restarted when the operation is complete.\r\n");
				return 1;
			}

			Rain::tsCout("\r\nStaging complete.\r\n");
			return 0;
		}
		int CHDeployStaging(CommandHandlerParam &cmhParam) {
			//request over the network
			ConnectionHandlerParam chParam;

			Rain::tsCout("Connecting to server...\r\n");
			fflush(stdout);
			Rain::ClientSocketManager csm;
			csm.setEventHandlers(onConnectionInit, onConnectionProcessMessage, onConnectionProcessMessage, &chParam);
			csm.setClientTarget((*cmhParam.config)["server-ip"], Rain::strToT<DWORD>((*cmhParam.config)["server-port-low"]), Rain::strToT<DWORD>((*cmhParam.config)["server-port-high"]));
			csm.blockForConnect(0);
			Rain::tsCout("Connected to server.\r\n");
			fflush(stdout);

			//authenticate with server
			cmhParam.logger->setSocketSrc(&csm, true);
			Rain::sendBlockMessage(csm, "authenticate " + (*cmhParam.config)["client-auth-pass"]);
			cmhParam.logger->setSocketSrc(&csm, false);
			Rain::sendBlockMessage(csm, "prod-stop");

			//refresh production files
			Rain::sendBlockMessage(csm, "prod-download");

			//upload files

			Sleep(100000);
			//cmhParam.logger->setSocketSrc(&csm, false);
			return 0;
		}
		int CHProdDownload(CommandHandlerParam &cmhParam) {
			return 0;
		}
		int CHStageProd(CommandHandlerParam &cmhParam) {
			return 0;
		}
		int CHProdStop(CommandHandlerParam &cmhParam) {
			return 0;
		}
		int CHProdStart(CommandHandlerParam &cmhParam) {
			return 0;
		}
		int CHSyncStop(CommandHandlerParam &cmhParam) {
			return 0;
		}
		int CHSyncStart(CommandHandlerParam &cmhParam) {
			return 0;
		}
	}
}