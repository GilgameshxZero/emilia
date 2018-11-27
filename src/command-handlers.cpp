#include "command-handlers.h"

namespace Emilia {
	int CHExit(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM != NULL) {
			delete cmhParam.remoteCSM;
			cmhParam.remoteCSM = NULL;
		}

		//return nonzero to exit program
		return 1;
	}
	int CHHelp(CommandHandlerParam &cmhParam) {
		//see readme for what each of these do
		Rain::tsCout("Available commands: exit, help, connect, disconnect, push, push-exclusive, pull, sync, start, stop, restart.", LINE_END);
		return 0;
	}

	int CHConnect(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM != NULL) {
			//already connected, don't allow another connect
			Rain::tsCout("Cannot execute 'connect' while already connected to remote.", LINE_END);
			return 0;
		}

		//prompt for more information
		std::string remoteAddr, passResponse, pass;
		Rain::tsCout("Prompt: Remote address: ");
		std::cin >> remoteAddr;
		Rain::tsCout("Prompt: Use configuration authentication? (y/n): ");
		std::cin >> passResponse;
		if (passResponse == "y") {
			pass = (*cmhParam.config)["emilia-auth-pass"];
		} else {
			Rain::tsCout("Prompt: Password for remote: ");
			std::cin >> pass;
			Rain::strTrimWhite(&pass);
		}

		//attempt to connect to remote
		Rain::tsCout("Info: Attempting to connect...", LINE_END);
		DWORD updateServerPort = Rain::strToT<DWORD>((*cmhParam.config)["update-server-port"]);
		cmhParam.remoteCSM = new Rain::ClientSocketManager();
		cmhParam.chParam = new UpdateClient::ConnectionHandlerParam();
		cmhParam.chParam->config = cmhParam.config;
		cmhParam.remoteCSM->setEventHandlers(UpdateClient::onConnect, UpdateClient::onMessage, UpdateClient::onDisconnect, cmhParam.chParam);
		cmhParam.remoteCSM->setClientTarget(remoteAddr, updateServerPort, updateServerPort);
		cmhParam.remoteCSM->blockForConnect(10000);

		int status = cmhParam.remoteCSM->getSocketStatus();
		if (status == 0) {
			Rain::tsCout("Info: Connected successfully!", LINE_END);
		} else if (status == 1) {
			Rain::tsCout("Error: Connection timeout.", LINE_END);
			delete cmhParam.remoteCSM;
			cmhParam.remoteCSM = NULL;
			return 0;
		} else {
			Rain::tsCout("Error: Disconnected.", LINE_END);
			delete cmhParam.remoteCSM;
			cmhParam.remoteCSM = NULL;
			return 0;
		}

		//authenticate
		Rain::tsCout("Info: Authenticating with update server...", LINE_END);
		cmhParam.chParam->waitingRequests++;
		ResetEvent(cmhParam.chParam->doneWaitingEvent);
		Rain::sendBlockMessage(*cmhParam.remoteCSM, "authenticate " + pass);

		//wait for response, then see if we are still connected
		WaitForSingleObject(cmhParam.chParam->doneWaitingEvent, 30000);
		if (cmhParam.chParam->lastSuccess != 0) {
			cmhParam.remoteCSM->setEventHandlers(NULL, NULL, NULL, NULL);
			cmhParam.remoteCSM->setClientTarget("", 0, 0);
			cmhParam.remoteCSM->blockForConnect();
			delete cmhParam.remoteCSM;
			cmhParam.remoteCSM = NULL;
			Rain::tsCout("Info: Update client is disconnected.", LINE_END);
			return 0;
		}

		return 0;
	}
	int CHDisconnect(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot disconnect when not connected.", LINE_END);
			return 0;
		}

		delete cmhParam.remoteCSM;
		cmhParam.remoteCSM = NULL;
		delete cmhParam.chParam;
		return 0;
	}
	int CHPush(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot execute this comment when not connected with remote.", LINE_END);
			return 0;
		}
		return 0;
	}
	int CHPushExclusive(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot execute this comment when not connected with remote.", LINE_END);
			return 0;
		}
		return 0;
	}
	int CHPull(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot execute this comment when not connected with remote.", LINE_END);
			return 0;
		}
		return 0;
	}
	int CHSync(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot execute this comment when not connected with remote.", LINE_END);
			return 0;
		}
		return 0;
	}
	int CHStart(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			if (!cmhParam.httpSM->setServerListen(80, 80)) {
				Rain::tsCout("HTTP server listening on port ", cmhParam.httpSM->getListeningPort(), ".", LINE_END);
			} else {
				DWORD error = GetLastError();
				Rain::errorAndCout(error, "Error: could not setup HTTP server listening.");
			}
			if (!cmhParam.smtpSM->setServerListen(25, 25)) {
				Rain::tsCout("SMTP server listening on port ", cmhParam.smtpSM->getListeningPort(), ".", LINE_END);
			} else {
				DWORD error = GetLastError();
				Rain::errorAndCout(error, "Error: could not setup SMTP server listening.");
			}
		} else {
			Rain::sendBlockMessage(*cmhParam.remoteCSM, "start");
		}
		return 0;
	}
	int CHStop(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			cmhParam.httpSM->setServerListen(0, 0);
			cmhParam.smtpSM->setServerListen(0, 0);
		} else {
			Rain::sendBlockMessage(*cmhParam.remoteCSM, "stop");
		}
		return 0;
	}
	int CHRestart(CommandHandlerParam &cmhParam) {
		CHStop(cmhParam);
		CHStart(cmhParam);
		return 0;
	}
	/*
	int CHStageDev(CommandHandlerParam &cmhParam) {
		//verify directory structures
		std::string devDir = (*cmhParam.config)["dev-root-dir"],
			stagingDir = (*cmhParam.config)["staging-root-dir"];
		if (!Rain::dirExists(devDir)) {
			Rain::tsCout("Failure: /Development/ not present\r\n");
			fflush(stdout);
			return 0;
		}
		if (!Rain::dirExists(stagingDir)) {
			Rain::tsCout("Failure: /Staging/ not present\r\n");
			fflush(stdout);
			return 0;
		}

		//remove everything in staging, except for those specified in staging-ignore
		Rain::tsCout("Info: Removing files in /Staging/, ignoring those specified in configuration...\r\n");
		fflush(stdout);
		std::vector<std::string> ignored;
		ignored = Rain::readMultilineFile((*cmhParam.config)["config-path"] + (*cmhParam.config)["staging-ignore"]);
		std::set<std::string> ignSet;
		for (int a = 0; a < ignored.size(); a++)
			ignSet.insert(Rain::pathToAbsolute(stagingDir + ignored[a]));
		Rain::rmDirRec(stagingDir, &ignSet);

		//stage relevant files
		Rain::tsCout("Info: Copying relevant files from /Development/ to /Staging/...\r\n\r\n");
		fflush(stdout);
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
						Rain::tsCout("Failure:\t\t");
					} else {
						Rain::tsCout("Copied:\t\t");
					}
				}
			}
			Rain::tsCout(file + "\r\n");
			fflush(stdout);
		}

		Rain::tsCout("\r\n");

		//if we need to replace the current script, run CRHelper, which will restart the current script when complete
		if (delayPath != "") {
			Rain::tsCout("Info: The current executable needs to be replaced. It will be restarted when the operation is complete.\r\n");
			fflush(stdout);

			std::string crhelperAbspath = Rain::pathToAbsolute((*cmhParam.config)["crhelper"]),
				crhWorkingDir = Rain::getPathDir(crhelperAbspath),
				crhCmdLine = "\"" + delayPath + "\" \"" +
				thisPath + "\" " +
				"stage-dev-crh-success";
			crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
			ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);
			return 1;
		}

		Rain::tsCout("Success: Staging complete.\r\n");
		fflush(stdout);
		return 0;
	}
	int CHDeployStaging(CommandHandlerParam &cmhParam) {
		//setup network stuff
		Rain::tsCout("Info: Connecting to server...\r\n");
		fflush(stdout);
		UpdateClient::ConnectionHandlerParam chParam;
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
		Rain::tsCout("Info: Copying /Staging to /Production, ignoring specified files.\r\n");
		fflush(stdout);

		//first step is to nuke production except ignored files
		std::vector<std::string> ignored;
		ignored = Rain::readMultilineFile((*cmhParam.config)["config-path"] + (*cmhParam.config)["deploy-ignore"]);
		std::set<std::string> ignSet;
		for (int a = 0; a < ignored.size(); a++)
			ignSet.insert(Rain::pathToAbsolute((*cmhParam.config)["prod-root-dir"] + ignored[a]));
		Rain::rmDirRec((*cmhParam.config)["prod-root-dir"], &ignSet);

		//copy staging to production, ignoring the same files
		ignSet.clear();
		for (int a = 0; a < ignored.size(); a++)
			ignSet.insert(Rain::pathToAbsolute((*cmhParam.config)["staging-root-dir"] + ignored[a]));
		Rain::cpyDirRec((*cmhParam.config)["staging-root-dir"], (*cmhParam.config)["prod-root-dir"], &ignSet);
		Rain::tsCout("Success: Copied /Staging to /Production, ignoring specified files.\r\n");
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
		std::string response = "prod-upload " + Rain::tToStr(files.size()) + "\r\n";
		bool filesReadable = true;
		for (int a = 0; a < files.size(); a++) {
			std::size_t fileSize = Rain::getFileSize((*cmhParam.config)["prod-root-dir"] + files[a]);
			if (fileSize == std::size_t(-1)) { //error
				filesReadable = false;
				break;
			}
			response += files[a] + "\r\n" + Rain::tToStr(fileSize) + "\r\n";
		}
		if (!filesReadable) { //should not have error here
			Rain::sendBlockMessage(csm, "prod-upload file-read-error");
			return 0;
		}

		//indicate we are starting the upload
		Rain::sendBlockMessage(csm, "prod-upload start");

		response += "\r\n";
		Rain::sendBlockMessage(csm, &response);

		Rain::tsCout("Info: ", files.size(), " files to upload.\r\n");
		fflush(stdout);

		std::size_t blockMax = Rain::strToT<std::size_t>((*cmhParam.config)["transfer-blocklen"]);
		for (int a = 0; a < files.size(); a++) {
			std::ifstream fileIn((*cmhParam.config)["prod-root-dir"] + files[a], std::ios::binary);
			std::size_t fileSize = Rain::getFileSize((*cmhParam.config)["prod-root-dir"] + files[a]);
			char *buffer = new char[blockMax];
			for (std::size_t b = 0; b < fileSize; b += blockMax) {
				Rain::tsCout("Info: Uploading file ", a + 1, " of ", files.size(), ". Done: ", b, " of ", fileSize, ".\r\n");
				fflush(stdout);

				fileIn.read(buffer, min(blockMax, fileSize - b));

				//send this buffer as data, with the prod-upload methodname, all in a single block
				Rain::sendBlockMessage(csm, "prod-upload " + std::string(buffer, min(blockMax, fileSize - b)));

				//TODO: why is this critical, even under TCP?
				csm.blockForMessageQueue();
			}
			delete[]buffer;
			Rain::tsCout("Info: Done uploading file ", a + 1, " of ", files.size(), ".\r\n");
			fflush(stdout);
			fileIn.close();
		}

		//indicate we are done with the file upload and waiting on a success code
		Rain::tsCout("Info: Upload done. Waiting for server confirmation...\r\n");
		fflush(stdout);
		chParam.waitingRequests = 1;
		ResetEvent(chParam.doneWaitingEvent);
		chParam.lastSuccess = -1;
		Rain::sendBlockMessage(csm, "prod-upload finish-success");
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
			Rain::tsCout("Info: Sending okay receipt to server, allowing it to restart...\r\n");
			Rain::sendBlockMessage(csm, "prod-upload restart-okay");
			fflush(stdout);
		}

		//replace all of staging with production: same routine as stage-prod
		return CHHStageProd(cmhParam, "deploy-staging-crh-success");
	}
	int CHProdDownload(CommandHandlerParam &cmhParam) {
		//setup network stuff
		Rain::tsCout("Connecting to server...\r\n");
		fflush(stdout);
		UpdateClient::ConnectionHandlerParam chParam;
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
		Rain::tsCout("Info: Connecting to server...\r\n");
		fflush(stdout);
		UpdateClient::ConnectionHandlerParam chParam;
		chParam.config = cmhParam.config;
		Rain::ClientSocketManager csm;
		if (CHHSetupCSM(cmhParam, csm, chParam))
			return 0;

		if (CHHProdDownload(cmhParam, csm, chParam))
			return 0;

		//no longer need network
		csm.~ClientSocketManager();

		return CHHStageProd(cmhParam, "stage-prod-crh-success");
	}
	int CHProdStop(CommandHandlerParam &cmhParam) {
		//setup network stuff
		Rain::tsCout("Info: Connecting to server...\r\n");
		fflush(stdout);
		UpdateClient::ConnectionHandlerParam chParam;
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
		Rain::tsCout("Info: Connecting to server...\r\n");
		fflush(stdout);
		UpdateClient::ConnectionHandlerParam chParam;
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
	
	int CHHSetupCSM(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam) {
		//all commands use the same handlers
		csm.setEventHandlers(UpdateClient::onConnect, UpdateClient::onMessage, UpdateClient::onDisconnect, &chParam);

		cmhParam.logger->setSocketSrc(&csm, true);

		//try connecting with each port in the range, and make sure our server is listening on the other end by authenticating with the port.
		DWORD lowPort = Rain::strToT<DWORD>((*cmhParam.config)["server-port-low"]),
			highPort = Rain::strToT<DWORD>((*cmhParam.config)["server-port-high"]);
		while (true) {
			csm.setClientTarget((*cmhParam.config)["server-ip"], lowPort, highPort);
			csm.blockForConnect(30000);
			if (csm.getSocketStatus() != csm.STATUS_CONNECTED) {
				Rain::tsCout("Failure: Error while connecting. Aborting...\r\n");
				fflush(stdout);
				return 1;
			}
			Rain::tsCout("Success: Connected to server.\r\n");
			fflush(stdout);

			//authenticate with server
			chParam.waitingRequests = 1;
			ResetEvent(chParam.doneWaitingEvent);
			chParam.lastSuccess = -1;
			Rain::sendBlockMessage(csm, "authenticate " + (*cmhParam.config)["emilia-auth-pass"]);
			WaitForSingleObject(chParam.doneWaitingEvent, 10000);
			if (chParam.lastSuccess) {
				if (csm.getConnectedPort() == highPort) {
					Rain::tsCout("Failure: Error while authenticating. No more ports to try. Aborting...\r\n");
					fflush(stdout);
					return 1;
				} else {
					Rain::tsCout("Info: Error while authenticating. Trying next port...\r\n");
					fflush(stdout);
					lowPort++;
					continue;
				}
			}
			Rain::tsCout("Success: Authenticated.\r\n");
			fflush(stdout);
			break;
		}

		//CSM does not need to send message before terminating connection

		cmhParam.logger->setSocketSrc(&csm, false);
		return 0;
	}

	int CHHProdDownload(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam) {
		Rain::tsCout("Info: Downloading server production to local production.\r\n");
		fflush(stdout);
		chParam.waitingRequests = 1;
		ResetEvent(chParam.doneWaitingEvent);
		chParam.lastSuccess = -1;
		Rain::sendBlockMessage(csm, "prod-download");
		WaitForSingleObject(chParam.doneWaitingEvent, INFINITE);
		if (chParam.lastSuccess) {
			Rain::tsCout("Failure: Error while downloading server production files. Aborting...\r\n");
			fflush(stdout);
			return 1;
		}
		Rain::tsCout("Success: Downloaded server production to local production.\r\n");
		fflush(stdout);

		return 0;
	}
	int CHHProdStop(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam) {
		cmhParam.logger->setSocketSrc(&csm, true);

		Rain::tsCout("Info: Stopping production servers...\r\n");
		fflush(stdout);
		chParam.waitingRequests = 1;
		ResetEvent(chParam.doneWaitingEvent);
		chParam.lastSuccess = -1;
		Rain::sendBlockMessage(csm, "prod-stop");
		WaitForSingleObject(chParam.doneWaitingEvent, INFINITE);
		if (chParam.lastSuccess) {
			Rain::tsCout("Failure: Error while stopping production servers. Aborting...\r\n");
			fflush(stdout);
			return 1;
		}
		Rain::tsCout("Success: Stopped production servers.\r\n");
		fflush(stdout);

		cmhParam.logger->setSocketSrc(&csm, false);
		return 0;
	}
	int CHHProdStart(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam) {
		cmhParam.logger->setSocketSrc(&csm, true);

		Rain::tsCout("Info: Starting production servers...\r\n");
		fflush(stdout);
		chParam.waitingRequests = 1;
		ResetEvent(chParam.doneWaitingEvent);
		chParam.lastSuccess = -1;
		Rain::sendBlockMessage(csm, "prod-start");
		WaitForSingleObject(chParam.doneWaitingEvent, INFINITE);
		if (chParam.lastSuccess) {
			Rain::tsCout("Failure: Error while starting production servers. Aborting...\r\n");
			fflush(stdout);
			return 1;
		}
		Rain::tsCout("Success: Started production servers.\r\n");
		fflush(stdout);

		cmhParam.logger->setSocketSrc(&csm, false);
		return 0;
	}
	int CHHSyncStop(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam) {
		return 0;
	}
	int CHHSyncStart(CommandHandlerParam &cmhParam, Rain::ClientSocketManager &csm, UpdateClient::ConnectionHandlerParam &chParam) {
		return 0;
	}

	int CHHStageProd(CommandHandlerParam &cmhParam, std::string restartCode) {
		//while replacing staging, we might need to restart this client with CRH
		Rain::tsCout("Info: Replacing /Staging with /Production...\r\n");
		std::vector<std::string> ignored;
		ignored = Rain::readMultilineFile((*cmhParam.config)["config-path"] + (*cmhParam.config)["deploy-ignore"]);
		std::set<std::string> ignSetStaging;
		for (int a = 0; a < ignored.size(); a++)
			ignSetStaging.insert(Rain::pathToAbsolute((*cmhParam.config)["staging-root-dir"] + ignored[a]));
		Rain::rmDirRec((*cmhParam.config)["staging-root-dir"], &ignSetStaging); //won't remove the current .exe, if in use

		//ignore some files while copying
		std::set<std::string> ignSetProd;
		for (int a = 0; a < ignored.size(); a++)
			ignSetProd.insert(Rain::pathToAbsolute((*cmhParam.config)["prod-root-dir"] + ignored[a]));
		Rain::cpyDirRec((*cmhParam.config)["prod-root-dir"], (*cmhParam.config)["staging-root-dir"], &ignSetProd);

		//check if we need to use CRH
		std::string exePath = Rain::pathToAbsolute(Rain::getExePath());
		std::string relExePath = exePath.substr(Rain::pathToAbsolute((*cmhParam.config)["staging-root-dir"]).length(), exePath.length());
		if (Rain::fileExists((*cmhParam.config)["prod-root-dir"] + relExePath)) {
			//need to use CRH
			Rain::tsCout("The current executable needs to be replaced. It will be restarted when the operation is complete.\r\n");

			std::string crhelperAbspath = Rain::pathToAbsolute((*cmhParam.config)["crhelper"]),
				crhWorkingDir = Rain::getPathDir(crhelperAbspath),
				crhCmdLine = "\"" + Rain::pathToAbsolute((*cmhParam.config)["prod-root-dir"] + relExePath) + "\" \"" + //src
				Rain::pathToAbsolute((*cmhParam.config)["staging-root-dir"] + relExePath) + "\" " + //dst
				restartCode;
			crhWorkingDir = crhWorkingDir.substr(0, crhWorkingDir.length() - 1);
			ShellExecute(NULL, "open", crhelperAbspath.c_str(), crhCmdLine.c_str(), crhWorkingDir.c_str(), SW_SHOWDEFAULT);
			return 1;
		}

		Rain::tsCout("Success: /Staging replaced with /Production...\r\n");
		fflush(stdout);
		return 0;
	}*/
}