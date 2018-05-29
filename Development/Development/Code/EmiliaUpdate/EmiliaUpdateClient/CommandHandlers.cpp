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
			std::vector<std::string> ignored;
			ignored = Rain::readMultilineFile((*cmhParam.config)["config-path"] + (*cmhParam.config)["staging-ignore"]);
			std::set<std::string> ignSet;
			for (int a = 0; a < ignored.size(); a++)
				ignSet.insert(stagingDir + ignored[a]);
			Rain::rmDirRec(stagingDir, &ignSet);

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
					"stage-dev-crh-success";
				crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
				ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);
				return 1;
			}

			Rain::tsCout("\r\nStaging complete.\r\n");
			return 0;
		}
		int CHDeployStaging(CommandHandlerParam &cmhParam) {
			//setup network stuff
			Rain::tsCout("Connecting to server...\r\n");
			fflush(stdout);
			ConnectionHandlerParam chParam;
			chParam.config = cmhParam.config;
			Rain::ClientSocketManager csm;
			if (CHHSetupCSM(cmhParam, csm, chParam))
				return 0;

			//stop the production servers
			if (CHHProdStop(cmhParam, csm, chParam))
				return 0;

			//refresh production files
			if (CHHProdDownload(cmhParam, csm, chParam))
				return 0;

			//replace production with staging, ignoring some files
			//first step is to nuke production except ignored files
			std::vector<std::string> ignored;
			ignored = Rain::readMultilineFile((*cmhParam.config)["config-path"] + (*cmhParam.config)["deploy-ignore"]);
			std::set<std::string> ignSet;
			for (int a = 0; a < ignored.size(); a++)
				ignSet.insert((*cmhParam.config)["prod-root-dir"] + ignored[a]);
			Rain::rmDirRec((*cmhParam.config)["prod-root-dir"], &ignSet);

			//copy staging to production, ignoring the same files
			Rain::cpyDirRec((*cmhParam.config)["staging-root-dir"], (*cmhParam.config)["prod-root-dir"], &ignSet);
			Rain::tsCout("Success: Copied staging to production, ignoring specified files.\r\n");
			fflush(stdout);

			//upload files to remote
			Rain::tsCout("Info: Uploading local production to server remote production.\r\n");
			fflush(stdout);
			chParam.waitingRequests = 1;
			ResetEvent(chParam.doneWaitingEvent);
			chParam.lastSuccess = -1;

			std::vector<std::string> files;
			files = Rain::getFilesRec((*cmhParam.config)["prod-root-dir"]);

			//send header as one block, then block the files based on a block-size limit
			std::string response = Rain::tToStr(files.size()) + "\r\n";
			bool filesReadable = true;
			for (int a = 0; a < files.size(); a++) {
				std::size_t fileSize = Rain::getFileSize((*cmhParam.config)["prod-root-dir"] + files[a]);
				if (fileSize == std::size_t(-1)) { //error
					filesReadable = false;
					break;
				}
				response += files[a] + "\r\n" + Rain::tToStr(fileSize) + "\r\n";
			}
			if (!filesReadable) {
				Rain::sendBlockMessage(csm, "prod-upload file-read-error");
				return 0;
			}

			//indicate we are starting the upload
			Rain::sendBlockMessage(csm, "prod-upload start");

			response += "\r\n";
			Rain::sendBlockMessage(csm, &response);

			std::size_t blockMax = Rain::strToT<std::size_t>((*cmhParam.config)["transfer-blocklen"]);
			for (int a = 0; a < files.size(); a++) {
				std::ifstream fileIn((*cmhParam.config)["prod-root-dir"] + files[a], std::ios::binary);
				std::size_t fileSize = Rain::getFileSize((*cmhParam.config)["prod-root-dir"] + files[a]);
				char *buffer = new char[blockMax];
				for (std::size_t b = 0; b < fileSize; b += blockMax) {
					fileIn.read(buffer, min(blockMax, fileSize - b));

					//send this buffer as data, with the prod-upload methodname, all in a single block
					Rain::sendBlockMessage(csm, "prod-upload" + std::string(buffer, min(blockMax, fileSize - b)));
				}
				fileIn.close();
			}

			//indicate we are done with the file upload
			Rain::sendBlockMessage(csm, "prod-upload finish-success");

			//wait for response from server for our prod-upload
			csm.blockForMessageQueue();
			WaitForSingleObject(chParam.doneWaitingEvent, INFINITE);
			if (chParam.lastSuccess == -1) {
				Rain::tsCout("Failure: Error while uploading production files. Aborting...\r\n");
				fflush(stdout);
				return 0;
			} else if (chParam.lastSuccess == 0) {
				Rain::tsCout("Success: Uploaded production files to remote.\r\n");
				fflush(stdout);
			} else if (chParam.lastSuccess == 1) {
				Rain::tsCout("Success: Uploaded production files to remote. Remote server needs to restart. Please reconnect later.\r\n");
				fflush(stdout);

				//wait to reconnect
				csm.blockForConnect(INFINITE);
				Rain::tsCout("Success: Reconnected to remote server.\r\n");
				fflush(stdout);
			}

			//start production servers
			if (CHHProdStart(cmhParam, csm, chParam))
				return 0;

			Rain::tsCout("Success: Command executed.\r\n");
			fflush(stdout);
			return 0;
		}
		int CHProdDownload(CommandHandlerParam &cmhParam) {
			//setup network stuff
			Rain::tsCout("Connecting to server...\r\n");
			fflush(stdout);
			ConnectionHandlerParam chParam;
			chParam.config = cmhParam.config;
			Rain::ClientSocketManager csm;
			if (CHHSetupCSM(cmhParam, csm, chParam))
				return 0;

			if (CHHProdDownload(cmhParam, csm, chParam))
				return 0;

			Rain::tsCout("Success: Command executed.\r\n");
			fflush(stdout);
			return 0;
		}
		int CHStageProd(CommandHandlerParam &cmhParam) {
			//a unique command which first downloads production from remote, then replaces staging with production, ignoring nothing

			//setup network stuff
			Rain::tsCout("Connecting to server...\r\n");
			fflush(stdout);
			ConnectionHandlerParam chParam;
			chParam.config = cmhParam.config;
			Rain::ClientSocketManager csm;
			if (CHHSetupCSM(cmhParam, csm, chParam))
				return 0;

			if (CHHProdDownload(cmhParam, csm, chParam))
				return 0;

			//no longer need network
			csm.~ClientSocketManager();

			//while replacing staging, we might need to restart this client with CRH
			Rain::tsCout("Info: Replacing /Staging with /Production...\r\n");
			Rain::rmDirRec((*cmhParam.config)["staging-root-dir"]); //won't remove the current .exe, if in use
			Rain::cpyDirRec((*cmhParam.config)["prod-root-dir"], (*cmhParam.config)["staging-root-dir"]);

			//check if we need to use CRH
			std::string exePath = Rain::pathToAbsolute(Rain::getExePath());
			std::string relExePath = exePath.substr(Rain::pathToAbsolute((*cmhParam.config)["staging-root-dir"]).length(), exePath.length());
			if (Rain::fileExists((*cmhParam.config)["prod-root-dir"] + relExePath)) {
				//need to use CRH
				Rain::tsCout("The current executable needs to be replaced. It will be restarted when the operation is complete.\r\n");

				std::string crhelperAbspath = Rain::pathToAbsolute((*cmhParam.config)["crhelper"]),
					crhWorkingDir = Rain::getPathDir(crhelperAbspath),
					crhCmdLine = "\"" + (*cmhParam.config)["prod-root-dir"] + relExePath + "\" \"" + //src
					(*cmhParam.config)["staging-root-dir"] + relExePath + "\"" + //dst
					"stage-prod-crh-success";
				crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
				ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);
				return 1;
			}

			Rain::tsCout("Success: /Staging replaced with /Production...\r\n");

			return 0;
		}
		int CHProdStop(CommandHandlerParam &cmhParam) {
			//setup network stuff
			Rain::tsCout("Connecting to server...\r\n");
			fflush(stdout);
			ConnectionHandlerParam chParam;
			chParam.config = cmhParam.config;
			Rain::ClientSocketManager csm;
			if (CHHSetupCSM(cmhParam, csm, chParam))
				return 0;

			if (CHHProdStop(cmhParam, csm, chParam))
				return 0;

			Rain::tsCout("Success: Command executed.\r\n");
			fflush(stdout);
			return 0;
		}
		int CHProdStart(CommandHandlerParam &cmhParam) {
			//setup network stuff
			Rain::tsCout("Connecting to server...\r\n");
			fflush(stdout);
			ConnectionHandlerParam chParam;
			chParam.config = cmhParam.config;
			Rain::ClientSocketManager csm;
			if (CHHSetupCSM(cmhParam, csm, chParam))
				return 0;

			if (CHHProdStart(cmhParam, csm, chParam))
				return 0;

			Rain::tsCout("Success: Command executed.\r\n");
			fflush(stdout);
			return 0;
		}
		int CHSyncStop(CommandHandlerParam &cmhParam) {
			return 0;
		}
		int CHSyncStart(CommandHandlerParam &cmhParam) {
			return 0;
		}

		int CHHSetupCSM(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam) {
			//all commands use the same handlers
			csm.setEventHandlers(onConnectionInit, onConnectionProcessMessage, onConnectionProcessMessage, &chParam);

			cmhParam.logger->setSocketSrc(&csm, true);

			//try connecting with each port in the range, and make sure our server is listening on the other end by authenticating with the port.
			DWORD lowPort = Rain::strToT<DWORD>((*cmhParam.config)["server-port-low"]),
				highPort = Rain::strToT<DWORD>((*cmhParam.config)["server-port-high"]);
			while (true) {
				csm.setClientTarget((*cmhParam.config)["server-ip"], lowPort, highPort);
				csm.blockForConnect(30000);
				if (csm.getSocketStatus() != csm.STATUS_CONNECTED) {
					Rain::tsCout("Error while connecting. Aborting...\r\n");
					fflush(stdout);
					return 1;
				}
				Rain::tsCout("Connected to server.\r\n");
				fflush(stdout);

				//authenticate with server
				chParam.waitingRequests = 1;
				ResetEvent(chParam.doneWaitingEvent);
				chParam.lastSuccess = -1;
				Rain::sendBlockMessage(csm, "authenticate " + (*cmhParam.config)["client-auth-pass"]);
				WaitForSingleObject(chParam.doneWaitingEvent, 10000);
				if (chParam.lastSuccess) {
					if (csm.getConnectedPort() == highPort) {
						Rain::tsCout("Error while authenticating. No more ports to try. Aborting...\r\n");
						fflush(stdout);
						return 1;
					} else {
						Rain::tsCout("Error while authenticating. Trying next port...\r\n");
						fflush(stdout);
						lowPort++;
						continue;
					}
				}
				Rain::tsCout("Success: Authenticated.\r\n");
				fflush(stdout);
			}

			//CSM does not need to send message before terminating connection

			cmhParam.logger->setSocketSrc(&csm, false);
			return 0;
		}

		int CHHProdDownload(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam) {
			Rain::tsCout("Info: Downloading server production to local production.\r\n");
			fflush(stdout);
			chParam.waitingRequests = 1;
			ResetEvent(chParam.doneWaitingEvent);
			chParam.lastSuccess = -1;
			Rain::sendBlockMessage(csm, "prod-download");
			WaitForSingleObject(chParam.doneWaitingEvent, INFINITE);
			if (chParam.lastSuccess) {
				Rain::tsCout("Error while downloading server production files. Aborting...\r\n");
				fflush(stdout);
				return 1;
			}
			Rain::tsCout("Success: Downloaded server production to local production.\r\n");
			fflush(stdout);

			return 0;
		}
		int CHHProdStop(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam) {
			cmhParam.logger->setSocketSrc(&csm, true);

			chParam.waitingRequests = 1;
			ResetEvent(chParam.doneWaitingEvent);
			chParam.lastSuccess = -1;
			Rain::sendBlockMessage(csm, "prod-stop");
			WaitForSingleObject(chParam.doneWaitingEvent, INFINITE);
			if (chParam.lastSuccess) {
				Rain::tsCout("Error while stopping production servers. Aborting...\r\n");
				fflush(stdout);
				return 1;
			}
			Rain::tsCout("Success: Stopped production servers.\r\n");
			fflush(stdout);

			cmhParam.logger->setSocketSrc(&csm, false);
			return 0;
		}
		int CHHProdStart(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam) {
			cmhParam.logger->setSocketSrc(&csm, true);

			chParam.waitingRequests = 1;
			ResetEvent(chParam.doneWaitingEvent);
			chParam.lastSuccess = -1;
			Rain::sendBlockMessage(csm, "prod-start");
			WaitForSingleObject(chParam.doneWaitingEvent, INFINITE);
			if (!chParam.lastSuccess) {
				Rain::tsCout("Error while starting production servers. Aborting...\r\n");
				fflush(stdout);
				return 1;
			}
			Rain::tsCout("Success: Started production servers.\r\n");
			fflush(stdout);

			cmhParam.logger->setSocketSrc(&csm, false);
			return 0;
		}
		int CHHSyncStop(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam) {
			return 0;
		}
		int CHHSyncStart(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, ConnectionHandlerParam &chParam) {
			return 0;
		}
	}
}