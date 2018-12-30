#include "main.h"

int main(int argc, char *argv[]) {
	//try to create a named mutex; if it already exists, then another instance of this application is running so terminate this
	//mutex name cannot have backslashes
	std::string mutexName = Rain::pathToAbsolute(Rain::getExePath());
	for (std::size_t a = 0; a < mutexName.length(); a++) {
		if (mutexName[a] == '\\') {
			mutexName[a] = '/';
		}
	}
	CreateMutex(NULL, FALSE, mutexName.c_str());
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		std::cout << "Another instance of application already running." << Rain::CRLF;
		return -1;
	}

	//restart the application if it didn't finish successfully
	if (FAILED(RegisterApplicationRestart(Rain::mbStrToWStr("crash-restart").c_str(), 0))) {
		std::cout << "RegisterApplicationRestart failed." << Rain::CRLF;
	}

	int error = Emilia::start(argc, argv);
	std::cout << "Start returned error code " << error << "." << Rain::CRLF;

	//finished successfully, so don't restart it
	UnregisterApplicationRestart();

	std::cout.flush();
	return error;
}

namespace Emilia {
	int start(int argc, char *argv[]) {
		//parameters
		std::map<std::string, std::string> config;

		std::string configFile = "../config/config.ini";
		Rain::concatMap(config, Rain::readParameterFile(configFile));

		//logging
		Rain::createDirRec(config["log-path"]);
		std::pair<std::streambuf *, std::ofstream *> cerrRedirect = Rain::redirectCerrFile(config["log-path"] + config["log-error"], true);
		HANDLE hFMemLeak = Rain::logMemoryLeaks(config["log-path"] + config["log-memory"]);

		Rain::LogStream logger;
		logger.setFileDst(config["log-path"] + config["log-log"], true);
		logger.setStdHandleSrc(STD_OUTPUT_HANDLE, true);

		//print parameters & command line
		Rain::tsCout("Emilia, version ", getVersionStr(), Rain::CRLF, Rain::CRLF);

		Rain::tsCout("Starting..." + Rain::CRLF + Rain::tToStr(config.size()), " configuration options:", Rain::CRLF);
		for (std::map<std::string, std::string>::iterator it = config.begin(); it != config.end(); it++) {
			Rain::tsCout("\t" + it->first + ": " + it->second + Rain::CRLF);
		}

		Rain::tsCout(Rain::CRLF, "Command line arguments: ", Rain::tToStr(argc), Rain::CRLF);
		for (int a = 0; a < argc; a++) {
			Rain::tsCout("\t", std::string(argv[a]) + Rain::CRLF);
		}
		Rain::tsCout(Rain::CRLF);

		//check command line for notifications
		if (argc >= 2) {
			std::string arg1 = argv[1];
			Rain::strTrimWhite(&arg1);
			if (arg1 == "update-restart") {
				Rain::tsCout("Important: Successfully restarted server after replacing from .tmp file from update operation.", Rain::CRLF);
			} else if (arg1 == "crash-restart") {
				Rain::tsCout("Important: Successfully recovering from crash.", Rain::CRLF);
			}
		}

		//http server setup
		HTTPServer::ConnectionCallerParam httpCCP;
		httpCCP.config = &config;
		httpCCP.logger = &logger;
		httpCCP.connectedClients = 0;

		std::ifstream cgiScriptsConfigIn(config["config-path"] + config["http-cgi-scripts"], std::ios::binary);
		while (cgiScriptsConfigIn.good()) {
			static std::string line;
			std::getline(cgiScriptsConfigIn, line);
			Rain::strTrimWhite(&line);
			if (line.length() > 0)
				//transform the cgi script paths into absolute paths
				httpCCP.cgiScripts.insert(Rain::pathToAbsolute(config["http-server-root"] + line));
		}
		cgiScriptsConfigIn.close();

		httpCCP.customHeaders = Rain::readParameterFile(config["config-path"] + config["http-custom-headers"]);
		httpCCP.customHeaders["server"] += " (version " + getVersionStr() + ")";

		if (Rain::fileExists(config["config-path"] + config["http-404"])) {
			Rain::readFileToStr(config["config-path"] + config["http-404"], httpCCP.notFound404HTML);
		}

		httpCCP.contentTypeSpec = Rain::readParameterFile(config["config-path"] + config["http-content-type"]);

		Rain::ServerManager httpSM;
		httpSM.setEventHandlers(HTTPServer::onConnect, HTTPServer::onMessage, HTTPServer::onDisconnect, &httpCCP);
		httpSM.setRecvBufLen(Rain::strToT<std::size_t>(config["http-transfer-buffer"]));

		//smtp server setup
		SMTPServer::ConnectionCallerParam smtpCCP;
		smtpCCP.config = &config;
		smtpCCP.logger = &logger;
		smtpCCP.connectedClients = 0;

		Rain::ServerManager smtpSM;
		smtpSM.setEventHandlers(SMTPServer::onConnect, SMTPServer::onMessage, SMTPServer::onDisconnect, &smtpCCP);
		smtpSM.setRecvBufLen(Rain::strToT<std::size_t>(config["smtp-transfer-buffer"]));

		//setup command handlers
		CommandHandlerParam cmhParam;
		cmhParam.config = &config;
		cmhParam.logger = &logger;
		cmhParam.httpSM = &httpSM;
		cmhParam.smtpSM = &smtpSM;

		cmhParam.excVec = Rain::readMultilineFile(config["config-path"] + config["update-exclusive-files"]);
		cmhParam.ignVec = Rain::readMultilineFile(config["config-path"] + config["update-ignore-files"]);
		cmhParam.excIgnVec = Rain::readMultilineFile(config["config-path"] + config["update-exc-ignore"]);
		cmhParam.ignVec.push_back(config["update-exclusive-dir"]);

		//format exclusives into absolute paths
		for (int a = 0; a < cmhParam.excVec.size(); a++) {
			cmhParam.excAbsSet.insert(Rain::pathToAbsolute(config["update-root"] + cmhParam.excVec[a]));
		}
		for (int a = 0; a < cmhParam.ignVec.size(); a++) {
			cmhParam.ignAbsSet.insert(Rain::pathToAbsolute(config["update-root"] + cmhParam.ignVec[a]));
		}
		cmhParam.notSharedAbsSet = cmhParam.excAbsSet;
		cmhParam.notSharedAbsSet.insert(cmhParam.ignAbsSet.begin(), cmhParam.ignAbsSet.end());

		//update server setup
		DWORD updateServerPort = Rain::strToT<DWORD>(config["update-server-port"]);

		UpdateServer::ConnectionCallerParam updCCP;
		updCCP.config = &config;
		updCCP.hInputExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		updCCP.clientConnected = false;
		updCCP.cmhParam = &cmhParam;

		Rain::HeadedServerManager updSM;
		updSM.setEventHandlers(UpdateServer::onConnect, UpdateServer::onMessage, UpdateServer::onDisconnect, &updCCP);
		updSM.setRecvBufLen(Rain::strToT<std::size_t>(config["update-transfer-buffer"]));
		if (!updSM.setServerListen(updateServerPort, updateServerPort)) {
			Rain::tsCout("Update server listening on port ", updSM.getListeningPort(), ".", Rain::CRLF);
		} else {
			DWORD error = GetLastError();
			Rain::errorAndCout(error, "Error: could not setup update server listening.");
		}

		//auto-start servers
		CHStart(cmhParam);

		//command loop
		std::mutex mtx;
		std::unique_lock<std::mutex> lc(mtx);
		std::map<std::string, CommandMethodHandler> commandHandlers{
			{"exit", CHExit},
			{"help", CHHelp},
			{"connect", CHConnect},
			{"disconnect", CHDisconnect},
			{"push", CHPush},
			{"push-exclusive", CHPushExclusive},
			{"pull", CHPull},
			{"sync", CHSync},
			{"start", CHStart},
			{"stop", CHStop},
			{"restart", CHRestart},
			{"restart-all", CHRestartAll}
		};
		while (true) {
			std::string command, tmp;
			Rain::tsCout("Accepting commands...", Rain::CRLF);
			std::cout.flush();

			std::cin >> command;
			Rain::strTrimWhite(command);
			std::getline(std::cin, tmp);

			auto handler = commandHandlers.find(command);
			if (handler != commandHandlers.end()) {
				if (handler->second(cmhParam) != 0) {
					break;
				}
			} else {
				Rain::tsCout("Command not recognized.", Rain::CRLF);
			}
		}

		Rain::tsCout("The program has terminated.", Rain::CRLF);
		std::cout.flush();

		logger.setStdHandleSrc(STD_OUTPUT_HANDLE, false);
		std::cerr.rdbuf(cerrRedirect.first);
		delete cerrRedirect.second;
		return 0;
	}
}