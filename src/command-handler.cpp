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
		Rain::tsCout("Available commands: exit, help, connect, disconnect, push, push-exclusive, pull, sync, start, stop, restart.", Rain::CRLF);
		return 0;
	}

	int CHConnect(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM != NULL) {
			//already connected, don't allow another connect
			Rain::tsCout("Cannot execute 'connect' while already connected to remote.", Rain::CRLF);
			return 0;
		}

		//prompt for more information
		std::string remoteAddr, passResponse, pass;
		Rain::tsCout("Prompt: Remote address: ");
		std::cin >> remoteAddr;
		Rain::tsCout("Prompt: Use configuration authentication? (y/n): ");
		std::cin >> passResponse;
		if (passResponse == "y") {
			//depending on the remote, take the password from the exclusive configuration for that remote; if not available, use the local password in the config
			std::string remoteCfgPath = Rain::pathToAbsolute((*cmhParam.config)["update-root"] + (*cmhParam.config)["update-exclusive-dir"] + remoteAddr + "\\config\\config.ini");
			if (Rain::fileExists(remoteCfgPath)) {
				pass = Rain::readParameterFile(remoteCfgPath)["emilia-auth-pass"];
			} else {
				pass = (*cmhParam.config)["emilia-auth-pass"];
			}
		} else {
			Rain::tsCout("Prompt: Password for remote: ");
			std::cin >> pass;
			Rain::strTrimWhite(&pass);
		}

		//attempt to connect to remote
		Rain::tsCout("Attempting to connect...", Rain::CRLF);
		DWORD updateServerPort = Rain::strToT<DWORD>((*cmhParam.config)["update-server-port"]);
		cmhParam.remoteCSM = new Rain::HeadedClientSocketManager();
		cmhParam.chParam = new UpdateClient::ConnectionHandlerParam();
		cmhParam.chParam->cmhParam = &cmhParam;
		cmhParam.chParam->config = cmhParam.config;
		cmhParam.chParam->authPass = pass;
		cmhParam.remoteCSM->setEventHandlers(UpdateClient::onConnect, UpdateClient::onMessage, UpdateClient::onDisconnect, cmhParam.chParam);
		cmhParam.remoteCSM->setClientTarget(remoteAddr, updateServerPort, updateServerPort);
		cmhParam.remoteCSM->blockForConnect(5000);

		int status = cmhParam.remoteCSM->getSocketStatus();
		if (status == 1) {
			Rain::tsCout("Error: Connection timeout.", Rain::CRLF);
			delete cmhParam.remoteCSM;
			cmhParam.remoteCSM = NULL;
			return 0;
		} else if (status != 0) {
			Rain::tsCout("Error: Could not connect.", Rain::CRLF);
			delete cmhParam.remoteCSM;
			cmhParam.remoteCSM = NULL;
			return 0;
		} //if status == 0 then we have connected successfully

		//wait for authentication response
		std::unique_lock<std::mutex> lck(cmhParam.chParam->authCV.getMutex());
		cmhParam.chParam->authCV.wait(lck);

		return 0;
	}
	int CHDisconnect(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot disconnect when not connected.", Rain::CRLF);
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
			Rain::tsCout("Cannot execute this command when not connected with remote.", Rain::CRLF);
			return 0;
		}

		//list all files marked as shared
		std::map<std::string, std::string> &config = *cmhParam.config;
		std::string root = Rain::pathToAbsolute(config["update-root"]);
		std::vector<std::string> shared = Rain::getFilesRec(root, "*", &cmhParam.notSharedAbsSet);
		Rain::tsCout("Found ", shared.size(), " shared files.", Rain::CRLF);
		std::cout.flush();

		//send over list of files and checksums
		Rain::sendHeadedMessage(*cmhParam.remoteCSM, "push" + UpdateHelper::generatePushHeader(root, shared));
		Rain::tsCout("Sending over 'push' request with hashes...", Rain::CRLF);
		std::cout.flush();

		//wait until the command is complete before accepting more commands
		std::unique_lock<std::mutex> lck(cmhParam.chParam->connectedCommandCV.getMutex());
		cmhParam.chParam->connectedCommandCV.wait(lck);

		return 0;
	}
	int CHPushExclusive(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot execute this command when not connected with remote.", Rain::CRLF);
			return 0;
		}

		//list all files marked as exclusive
		std::map<std::string, std::string> &config = *cmhParam.config;
		std::string root = Rain::pathToAbsolute(config["update-root"]),
			ip = cmhParam.remoteCSM->getTargetIP();
		std::string excRoot = root + config["update-exclusive-dir"] + ip + "\\";
		std::set<std::string> want;
		for (std::size_t a = 0; a < cmhParam.excVec.size(); a++) {
			want.insert(excRoot + cmhParam.excVec[a]);
		}
		std::set<std::string> excIgnAbsSet;
		for (std::size_t a = 0; a < cmhParam.excIgnVec.size(); a++) {
			excIgnAbsSet.insert(excRoot + cmhParam.excIgnVec[a]);
		}
		std::vector<std::string> exclusive = Rain::getFilesRec(excRoot, "*", &excIgnAbsSet, &want);
		Rain::tsCout("Found ", exclusive.size(), " exclusive files for '", ip, "'.", Rain::CRLF);
		std::cout.flush();

		//send over list of files and checksums
		Rain::sendHeadedMessage(*cmhParam.remoteCSM, "push-exclusive" + UpdateHelper::generatePushHeader(excRoot, exclusive));
		Rain::tsCout("Sending over 'push-exclusive' request with hashes...", Rain::CRLF);
		std::cout.flush();

		//wait until the command is complete before accepting more commands
		std::unique_lock<std::mutex> lck(cmhParam.chParam->connectedCommandCV.getMutex());
		cmhParam.chParam->connectedCommandCV.wait(lck);

		return 0;
	}
	int CHPull(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			//not yet connected
			Rain::tsCout("Cannot execute this command when not connected with remote.", Rain::CRLF);
			return 0;
		}

		//send request to remote for headers
		Rain::sendHeadedMessage(*cmhParam.remoteCSM, "pull ");
		Rain::tsCout("Sending `pull` request...", Rain::CRLF);
		std::cout.flush();

		//wait for command to finish
		std::unique_lock<std::mutex> lck(cmhParam.chParam->connectedCommandCV.getMutex());
		cmhParam.chParam->connectedCommandCV.wait(lck);

		return 0;
	}
	int CHSync(CommandHandlerParam &cmhParam) {
		CHPush(cmhParam);
		CHPushExclusive(cmhParam);
		CHPull(cmhParam);
		return 0;
	}
	int CHStart(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			if (!cmhParam.httpSM->setServerListen(80, 80)) {
				Rain::tsCout("HTTP server listening on port ", cmhParam.httpSM->getListeningPort(), ".", Rain::CRLF);
			} else {
				DWORD error = GetLastError();
				Rain::errorAndCout(error, "Error: could not setup HTTP server listening.");
			}
			if (!cmhParam.smtpSM->setServerListen(25, 25)) {
				Rain::tsCout("SMTP server listening on port ", cmhParam.smtpSM->getListeningPort(), ".", Rain::CRLF);
			} else {
				DWORD error = GetLastError();
				Rain::errorAndCout(error, "Error: could not setup SMTP server listening.");
			}
			std::cout.flush();
		} else {
			Rain::sendHeadedMessage(*cmhParam.remoteCSM, "start");

			//wait for command to finish
			std::unique_lock<std::mutex> lck(cmhParam.chParam->connectedCommandCV.getMutex());
			cmhParam.chParam->connectedCommandCV.wait(lck);
		}
		return 0;
	}
	int CHStop(CommandHandlerParam &cmhParam) {
		if (cmhParam.remoteCSM == NULL) {
			cmhParam.httpSM->setServerListen(0, 0);
			cmhParam.smtpSM->setServerListen(0, 0);
			Rain::tsCout("HTTP & SMTP servers stopped.", Rain::CRLF);
			std::cout.flush();
		} else {
			Rain::sendHeadedMessage(*cmhParam.remoteCSM, "stop");

			//wait for command to finish
			std::unique_lock<std::mutex> lck(cmhParam.chParam->connectedCommandCV.getMutex());
			cmhParam.chParam->connectedCommandCV.wait(lck);
		}
		return 0;
	}
	int CHRestart(CommandHandlerParam &cmhParam) {
		CHStop(cmhParam);
		CHStart(cmhParam);
		return 0;
	}
	int CHRestartAll(CommandHandlerParam &cmhParam) {
		std::map<std::string, std::string> &config = *cmhParam.config;

		if (cmhParam.remoteCSM == NULL) {
			std::string updateScript = Rain::pathToAbsolute(config["update-root"] + config["update-script"]),
				serverPath = "\"" + Rain::pathToAbsolute(Rain::getExePath()) + "\"";
			ShellExecute(NULL, "open", updateScript.c_str(),
				(serverPath + " " + serverPath).c_str(),
				Rain::getPathDir(updateScript).c_str(), SW_SHOWDEFAULT);

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			exit(1);
		} else {
			Rain::sendHeadedMessage(*cmhParam.remoteCSM, "restart-all");

			//wait for re-authenticate
			std::unique_lock<std::mutex> lck(cmhParam.chParam->authCV.getMutex());
			cmhParam.chParam->authCV.wait(lck);
		}

		return 0;
	}
}