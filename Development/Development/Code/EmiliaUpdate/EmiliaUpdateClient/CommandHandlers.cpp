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
			std::string devDir = (*cmhParam.config)["dev-root-dir"],
				stagingDir = (*cmhParam.config)["staging-root-dir"];
			if (!Rain::dirExists(devDir)) {
				Rain::tsCout("/Development/ not present\r\n");
				return 0;
			}
			if (!Rain::dirExists(stagingDir)) {
				Rain::tsCout("/Staging/ not present\r\n");
				return 0;
			}

			//remove everything in staging, except for those specified in staging-ignore
			Rain::tsCout("Removing files in /Staging/, ignoring those specified in configuration...\r\n");
			std::vector<std::string> ignoredFiles;
			ignoredFiles = Rain::readMultilineFile((*cmhParam.config)["config-path"] + (*cmhParam.config)["staging-ignore"]);
			Rain::recursiveRmDir(stagingDir, &ignoredFiles);

			//stage relevant files
			Rain::tsCout("Copying relevant files from /Development/ to /Staging/...\r\n\r\n");
			std::vector<std::string> stagingFiles;
			stagingFiles = Rain::readMultilineFile((*cmhParam.config)["config-path"] + (*cmhParam.config)["staging-files"]);

			//if we are replacing the current executable, then delay the replace until after the program exits
			std::string thisPath = Rain::pathToAbsolute(Rain::getExePath()),
				delayPath = "";

			for (std::string file : stagingFiles) {
				if (!Rain::fileExists(devDir + file)) {
					Rain::tsCout("Doesn't exist:\t");
				} else {
					if (thisPath == Rain::pathToAbsolute(stagingDir + file)) {
						delayPath = Rain::pathToAbsolute(devDir + file);
						Rain::tsCout("Delayed:\t");
					} else {
						Rain::createDirRec(Rain::getPathDir(stagingDir + file));
						if (!CopyFile((devDir + file).c_str(), (stagingDir + file).c_str(), FALSE)) {
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
				Rain::tsCout("\r\nThe current executable needs to be replaced. It will be restarted when the operation is complete.\r\n");

				std::string crhelperAbspath = Rain::pathToAbsolute((*cmhParam.config)["crhelper"]),
					crhWorkingDir = Rain::getPathDir(crhelperAbspath),
					crhCmdLine = "\"" + delayPath + "\" \"" +
					thisPath + "\"" +
					"staging-crh-success";
				crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
				ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);
				return 1;
			}

			Rain::tsCout("\r\nStaging complete.\r\n");
			return 0;
		}
		int CHDeployStaging(CommandHandlerParam &cmhParam) {
			//request over the network
			ConnectionHandlerParam chParam;
			chParam.config = cmhParam.config;

			Rain::tsCout("Connecting to server...\r\n");
			fflush(stdout);
			Rain::ClientSocketManager csm;
			csm.setEventHandlers(onConnectionInit, onConnectionProcessMessage, onConnectionProcessMessage, &chParam);
			csm.setClientTarget((*cmhParam.config)["server-ip"], Rain::strToT<DWORD>((*cmhParam.config)["server-port-low"]), Rain::strToT<DWORD>((*cmhParam.config)["server-port-high"]));
			csm.blockForConnect(INFINITE);
			Rain::tsCout("Connected to server.\r\n");
			fflush(stdout);

			//authenticate with server
			cmhParam.logger->setSocketSrc(&csm, true);
			chParam.waitingRequests = 1;
			ResetEvent(chParam.doneWaitingEvent);
			chParam.lastSuccess = false;
			Rain::sendBlockMessage(csm, "authenticate " + (*cmhParam.config)["client-auth-pass"]);
			WaitForSingleObject(chParam.doneWaitingEvent, INFINITE);
			if (!chParam.lastSuccess) {
				Rain::tsCout("Error while authenticating. Aborting...\r\n");
				return 0;
			}
			Rain::tsCout("SUCCESS: Authenticated.\r\n");
			fflush(stdout);

			//stop the production servers
			chParam.waitingRequests = 1;
			ResetEvent(chParam.doneWaitingEvent);
			chParam.lastSuccess = false;
			Rain::sendBlockMessage(csm, "prod-stop");
			WaitForSingleObject(chParam.doneWaitingEvent, INFINITE);
			if (!chParam.lastSuccess) {
				Rain::tsCout("Error while stopping production servers. Aborting...\r\n");
				return 0;
			}
			Rain::tsCout("SUCCESS: Stopped production servers.\r\n");
			cmhParam.logger->setSocketSrc(&csm, false);
			fflush(stdout);

			//refresh production files
			chParam.waitingRequests = 1;
			ResetEvent(chParam.doneWaitingEvent);
			chParam.lastSuccess = false;
			Rain::sendBlockMessage(csm, "prod-download");
			WaitForSingleObject(chParam.doneWaitingEvent, INFINITE);
			if (!chParam.lastSuccess) {
				Rain::tsCout("Error while downloading server production files. Aborting...\r\n");
				return 0;
			}
			Rain::tsCout("SUCCESS: Downloaded server production to local production.\r\n");
			fflush(stdout);

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