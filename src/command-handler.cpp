#include "command-handler.h"

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
		cmhParam.remoteCSM = new Rain::HeadedClientSocketManager();
		cmhParam.chParam = new UpdateClient::ConnectionHandlerParam();
		cmhParam.chParam->config = cmhParam.config;
		cmhParam.chParam->authPass = pass;
		cmhParam.chParam->doneWaitingEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		ResetEvent(cmhParam.chParam->doneWaitingEvent);
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
			Rain::tsCout("Cannot execute this command when not connected with remote.", LINE_END);
			return 0;
		}

		//list all files marked as shared
		std::map<std::string, std::string> &config = *cmhParam.config;
		std::string root = Rain::pathToAbsolute(config["update-root"]);
		std::vector<std::string> shared = Rain::getFilesRec(root, "*", &cmhParam.notSharedAbsSet);
		Rain::tsCout("Found ", shared.size(), " shared files.", LINE_END);
		fflush(stdout);

		//send over list of files and checksums
		Rain::sendHeadedMessage(*cmhParam.remoteCSM, "push" + CHHPushGenerateRequest(root, shared));
		Rain::tsCout("Sending over 'push' request with checksums...", LINE_END);
		fflush(stdout);

		return 0;
	}
	int CHPushExclusive(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot execute this command when not connected with remote.", LINE_END);
			return 0;
		}

		//list all files marked as exclusive
		std::map<std::string, std::string> &config = *cmhParam.config;
		std::string root = Rain::pathToAbsolute(config["update-root"]),
			ip = cmhParam.remoteCSM->getTargetIP();
		std::string excRoot = root + config["update-exclusive-dir"] + ip + "\\";
		std::set<std::string> want;
		for (int a = 0; a < cmhParam.excVec.size(); a++) {
			want.insert(excRoot + cmhParam.excVec[a]);
		}
		std::vector<std::string> exclusive = Rain::getFilesRec(excRoot, "*", NULL, &want);
		Rain::tsCout("Found ", exclusive.size(), " exclusive files for '", ip, "'.", LINE_END);
		fflush(stdout);

		//send over list of files and checksums
		Rain::sendHeadedMessage(*cmhParam.remoteCSM, "push-exclusive" + CHHPushGenerateRequest(excRoot, exclusive));
		Rain::tsCout("Sending over 'push-exclusive' request with checksums...", LINE_END);
		fflush(stdout);

		return 0;
	}
	int CHPull(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot execute this command when not connected with remote.", LINE_END);
			return 0;
		}

		//send request to remote for headers

		return 0;
	}
	int CHSync(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot execute this command when not connected with remote.", LINE_END);
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
			Rain::sendHeadedMessage(*cmhParam.remoteCSM, "start");
		}
		return 0;
	}
	int CHStop(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			cmhParam.httpSM->setServerListen(0, 0);
			cmhParam.smtpSM->setServerListen(0, 0);
		} else {
			Rain::sendHeadedMessage(*cmhParam.remoteCSM, "stop");
		}
		return 0;
	}
	int CHRestart(CommandHandlerParam &cmhParam) {
		CHStop(cmhParam);
		CHStart(cmhParam);
		return 0;
	}

	std::string CHHPushGenerateRequest(std::string root, std::vector<std::string> &files) {
		//generate hashes (using last write time instead of crc32)
		std::vector<FILETIME> hash(files.size());
		Rain::tsCout(std::hex);
		for (int a = 0; a < files.size(); a++) {
			HANDLE hFile;
			hFile = CreateFile((root + files[a]).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			GetFileTime(hFile, NULL, NULL, &hash[a]);
			CloseHandle(hFile);

			Rain::tsCout(std::setw(8), hash[a].dwHighDateTime, std::setw(8), hash[a].dwLowDateTime, " ", files[a], LINE_END);
			fflush(stdout);
		}
		Rain::tsCout(std::dec);

		std::string message = " " + Rain::tToStr(files.size()) + "\n";
		for (int a = 0; a < files.size(); a++) {
			message += Rain::tToStr(hash[a].dwHighDateTime) + " " + Rain::tToStr(hash[a].dwLowDateTime) + " " + files[a] + "\n";
		}

		return message;
	}
}