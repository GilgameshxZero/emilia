#include "command-handler.hpp"

namespace Emilia {
	namespace CommandHandler {
		int exit(MainParam &mp) {
			if (mp.remoteCSM != NULL) {
				delete mp.remoteCSM;
				mp.remoteCSM = NULL;
			}
			return 1;
		}
		int connect(MainParam &mp) {
			if (mp.remoteCSM != NULL && mp.remoteCSM->getSocketStatus() == -1) {
				disconnect(mp);
			}
			if (mp.remoteCSM != NULL) {
				Rain::tsCout("Cannot execute 'connect' while already connected to remote.", Rain::CRLF);
				return 0;
			}

			Rain::Configuration &config = *mp.config;

			//prompt for more information
			std::string remoteAddr, cfgResponse, pass;
			DWORD port;

			Rain::tsCout("Remote address: ");
			std::cout.flush();
			std::cin >> remoteAddr;
			Rain::tsCout("Use current project configuration authentication? (y/n): ");
			std::cout.flush();
			std::cin >> cfgResponse;

			if (cfgResponse == "y") {
				pass = config["deploy-pw"].s();
				port = config["deploy-port"].i();
			} else {
				Rain::tsCout("Remote password: ");
				std::cout.flush();
				std::cin >> pass;
				Rain::strTrimWhite(&pass);
				std::string s;
				Rain::tsCout("Remote port: ");
				std::cout.flush();
				std::cin >> s;
				port = Rain::strToT<DWORD>(s);
			}

			//attempt to connect to remote
			static const int CONNECT_TO_MS = 5000;
			Rain::tsCout("Connecting (", CONNECT_TO_MS, "ms timeout)...", Rain::CRLF);
			mp.remoteCSM = new Rain::HeadedClientSocketManager();
			mp.deployCHP = new DeployClient::ConnectionHandlerParam();
			mp.deployCHP->project = mp.project;
			mp.deployCHP->config = mp.config;
			mp.deployCHP->logDeploy = mp.logDeploy;
			mp.deployCHP->httpSM = &mp.httpSM;
			mp.deployCHP->smtpSM = &mp.smtpSM;
			mp.deployCHP->authPass = pass;
			mp.remoteCSM->setEventHandlers(DeployClient::onConnect, DeployClient::onMessage, DeployClient::onDisconnect, mp.deployCHP);
			mp.remoteCSM->setClientTarget(remoteAddr, port, port);
			mp.remoteCSM->blockForConnect(3000);

			int status = mp.remoteCSM->getSocketStatus();
			if (status == 1) {
				Rain::tsCout("Connection timeout.", Rain::CRLF);
				delete mp.remoteCSM;
				mp.remoteCSM = NULL;
				return 0;
			} else if (status != 0) {
				Rain::tsCout("Could not connect to remote server.", Rain::CRLF);
				delete mp.remoteCSM;
				mp.remoteCSM = NULL;
				return 0;
			} //if status == 0 then we have connected successfully

			//wait for authentication response
			std::unique_lock<std::mutex> lck(mp.deployCHP->authCV.getMutex());
			mp.deployCHP->authCV.wait(lck);

			return 0;
		}
		int disconnect(MainParam &mp) {
			if (mp.remoteCSM == NULL) {
				Rain::tsCout("Cannot disconnect when not connected.", Rain::CRLF);
				return 0;
			}

			delete mp.remoteCSM;
			mp.remoteCSM = NULL;
			delete mp.deployCHP;
			return 0;
		}
		int server(MainParam &mp) {
			if (mp.remoteCSM != NULL) {
				Rain::tsCout("Connected to remote ", mp.remoteCSM->getTargetIP(), ".", Rain::CRLF);
				Rain::sendHeadedMessage(*mp.remoteCSM, "server");

				//wait for command to finish
				std::unique_lock<std::mutex> lck(mp.deployCHP->connectedCommandCV.getMutex());
				mp.deployCHP->connectedCommandCV.wait(lck);
			} else {
				Rain::tsCout("Not connected to remote.", Rain::CRLF);
				if (mp.httpSM.getListeningPort() != -1) {
					Rain::tsCout("HTTP server listening on port ", mp.httpSM.getListeningPort(), ".", Rain::CRLF);
				} else {
					Rain::tsCout("HTTP server not listening.", Rain::CRLF);
				}
				if (mp.smtpSM.getListeningPort() != -1) {
					Rain::tsCout("SMTP server listening on port ", mp.smtpSM.getListeningPort(), ".", Rain::CRLF);
				} else {
					Rain::tsCout("SMTP server not listening.", Rain::CRLF);
				}
			}

			std::string command;
			Rain::tsCout("Type 'start', 'stop', or anything else to return to main menu: ");
			std::cin >> command;
			Rain::strTrimWhite(&command);

			if (command == "start") {
				if (mp.remoteCSM == NULL) {
					startServers(mp, 3);
				} else {
					Rain::sendHeadedMessage(*mp.remoteCSM, "server start");

					//wait for command to finish
					std::unique_lock<std::mutex> lck(mp.deployCHP->connectedCommandCV.getMutex());
					mp.deployCHP->connectedCommandCV.wait(lck);
				}
			} else if (command == "stop") {
				if (mp.remoteCSM == NULL) {
					mp.httpSM.setServerListen(0, 0);
					mp.smtpSM.setServerListen(0, 0);
					Rain::tsCout("HTTP & SMTP servers stopped.", Rain::CRLF);
					std::cout.flush();
				} else {
					Rain::sendHeadedMessage(*mp.remoteCSM, "server stop");

					//wait for command to finish
					std::unique_lock<std::mutex> lck(mp.deployCHP->connectedCommandCV.getMutex());
					mp.deployCHP->connectedCommandCV.wait(lck);
				}
			}

			return 0;
		}
		int restart(MainParam &mp) {
			Rain::Configuration &config = *mp.config;

			if (mp.remoteCSM == NULL) {
				prepRestart(mp.project, &mp.httpSM, &mp.smtpSM);
				return 1;
			} else {
				mp.remoteCSM->setRetryOnDisconnect(true);
				Rain::sendHeadedMessage(*mp.remoteCSM, "restart");

				//wait for re-authenticate
				std::unique_lock<std::mutex> lck(mp.deployCHP->authCV.getMutex());
				mp.deployCHP->authCV.wait(lck);
			}

			return 0;
		}
		int project(MainParam &mp) {
			Rain::tsCout("Project: ", mp.project, Rain::CRLF);
			Rain::tsCout("Would you like to switch projects? (y/n): ");
			std::string prompt;
			std::cin >> prompt;

			if (prompt == "y") {
				std::string project;

				Rain::tsCout("Directory to project: ");
				std::cin >> prompt;
				Rain::standardizeDirPath(&prompt);
				prompt = Rain::pathToAbsolute(prompt);
				Rain::createDirRec(prompt);

				//check if path is already a project
				std::vector<std::string> dirs = Rain::getDirs(prompt);
				bool isProject = false;
				for (int a = 0; a < dirs.size(); a++) {
					if (dirs[a] == ".emilia") {
						isProject = true;
						break;
					}
				}

				if (!isProject) {
					Rain::tsCout("The specified directory is not a project. Would you like to create one? (y/n): ");
					std::string create;
					std::cin >> create;
					if (create == "y") {
						initProjectDir(prompt);
						project = prompt;
					} else {
						Rain::tsCout("Aborting operation...", Rain::CRLF);
						return 0;
					}
				} else {
					project = prompt;
				}

				//switch out configuration options and everything else associated with them
				freeMainParam(mp);
				initMainParam(project, mp);
			}

			return 0;
		}
		int sync(MainParam &mp) {
			if (mp.remoteCSM != NULL && mp.remoteCSM->getSocketStatus() == -1) {
				disconnect(mp);
			}
			if (mp.remoteCSM == NULL) {
				Rain::tsCout("Cannot sync when not connected.", Rain::CRLF);
				return 0;
			}

			Rain::sendHeadedMessage(*mp.remoteCSM, "sync");

			//wait for command to finish
			std::unique_lock<std::mutex> lck(mp.deployCHP->connectedCommandCV.getMutex());
			mp.deployCHP->connectedCommandCV.wait(lck);

			return 0;
		}
	}
}