#include "main.h"

const std::string LINE_END = "\r\n";

int main(int argc, char* argv[]) {
	//restart the application if it didn't finish successfully
	RegisterApplicationRestart(Rain::mbStrToWStr("crash-restart").c_str(), 0);

	int error;
	error = Emilia::start(argc, argv);
	std::cout << "start returned error code " << error << "." << LINE_END;

	//finished successfully, so don't restart it
	UnregisterApplicationRestart();

	fflush(stdout);
	return error;
}

namespace Emilia {
	int start(int argc, char* argv[]) {
		//parameters
		std::map<std::string, std::string> config;

		std::string configFile = "../config/config.ini";
		Rain::concatMap(config, Rain::readParameterFile(configFile));

		//logging
		Rain::createDirRec(config["log-path"]);
		Rain::redirectCerrFile(config["log-path"] + config["log-error"], true);
		HANDLE hFMemLeak = Rain::logMemoryLeaks(config["log-path"] + config["log-memory"]);

		Rain::LogStream logger;
		logger.setFileDst(config["log-path"] + config["log-log"], true);
		logger.setStdHandleSrc(STD_OUTPUT_HANDLE, true);

		//print parameters & command line
		Rain::tsCout("Starting...\r\n" + Rain::tToStr(config.size()) + " configuration options:\r\n");
		for (std::map<std::string, std::string>::iterator it = config.begin(); it != config.end(); it++) {
			Rain::tsCout("\t" + it->first + ": " + it->second + "\r\n");
		}

		Rain::tsCout("\r\nCommand line arguments: " + Rain::tToStr(argc) + "\r\n\r\n");
		for (int a = 0; a < argc; a++) {
			Rain::tsCout(std::string(argv[a]) + "\r\n");
		}
		Rain::tsCout("\r\n");

		//setup command handlers
		static std::map<std::string, CommandMethodHandler> commandHandlers{
			{"exit", CHExit},
			{"help", CHHelp},
			{"stage-dev", CHStageDev},
			{"deploy-staging", CHDeployStaging},
			{"prod-download", CHProdDownload},
			{"stage-prod", CHStageProd},
			{"prod-stop", CHProdStop},
			{"prod-start", CHProdStart},
			{"sync-stop", CHSyncStop},
			{"sync-start", CHSyncStart},
		};
		CommandHandlerParam cmhParam;
		cmhParam.config = &config;
		cmhParam.logger = &logger;

		//check command line for notifications
		if (argc >= 2) {
			std::string arg1 = argv[1];
			Rain::strTrimWhite(&arg1);
			if (arg1 == "prod-upload-success") {
				Rain::tsCout("Important: 'prod-upload' CRH operation completed successfully.\r\n");

				//remove tmp file of the prod-upload operation
				std::string filePath = Rain::pathToAbsolute(Rain::getExePath() + config["update-tmp-ext"]);
				DeleteFile(filePath.c_str());
				Rain::tsCout("Temporary file for 'prod-upload' deleted.\r\n\r\n");
			} else if (arg1 == "crash-restart") {
				Rain::tsCout("Important: Server recovering from crash.\r\n");
			} else if (arg1 == "stage-dev-crh-success")
				Rain::tsCout("Important: 'stage-dev' CRH completed successfully.\r\n");
			else if (arg1 == "stage-prod-crh-success")
				Rain::tsCout("Important: 'stage-prod' CRH completed successfully.\r\n");
			else if (arg1 == "deploy-staging-crh-success") {
				Rain::tsCout("Important: 'deploy-staging' CRH completed successfully.\r\n");
				CHProdStart(cmhParam);
			}
		}

		//update server setup
		DWORD updateServerPort = Rain::strToT<DWORD>(config["update-server-port"]);

		UpdateServer::ConnectionCallerParam updCCP;
		updCCP.config = &config;
		updCCP.hInputExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		updCCP.serverStatus.resize(0); //separate servers deprecated for now
		updCCP.clientConnected = false;

		Rain::ServerManager updSM;
		updSM.setEventHandlers(UpdateServer::onConnect, UpdateServer::onMessage, UpdateServer::onDisconnect, &updCCP);
		updSM.setRecvBufLen(Rain::strToT<std::size_t>(config["update-transfer-buffer"]));
		if (!updSM.setServerListen(updateServerPort, updateServerPort)) {
			Rain::tsCout("Update server listening on port ", updSM.getListeningPort(), ".\r\n");
		} else {
			Rain::tsCout("Fatal error: could not setup update server listening.\r\n");
			DWORD error = GetLastError();
			Rain::reportError(error, "Fatal error: could not setup update server listening.");
			WSACleanup();
			if (hFMemLeak != NULL)
				CloseHandle(hFMemLeak);
			return error;
		}

		//http server setup
		HTTPServer::ConnectionCallerParam httpCCP;
		httpCCP.config = &config;
		httpCCP.logger = &logger;
		httpCCP.connectedClients = 0;

		Rain::ServerManager httpSM;
		httpSM.setEventHandlers(HTTPServer::onConnect, HTTPServer::onMessage, HTTPServer::onDisconnect, &httpCCP);
		httpSM.setRecvBufLen(Rain::strToT<std::size_t>(config["http-transfer-buffer"]));
		if (!httpSM.setServerListen(80, 80)) {
			Rain::tsCout("HTTP server listening on port ", httpSM.getListeningPort(), ".\r\n");
		} else {
			Rain::tsCout("Fatal error: could not setup HTTP server listening.\r\n");
			fflush(stdout);
			DWORD error = GetLastError();
			Rain::reportError(error, "Fatal error: could not setup HTTP server listening.");
			WSACleanup();
			if (hFMemLeak != NULL)
				CloseHandle(hFMemLeak);
			return error;
		}

		//smtp server setup
		SMTPServer::ConnectionCallerParam smtpCCP;
		smtpCCP.config = &config;
		smtpCCP.logger = &logger;
		smtpCCP.connectedClients = 0;

		Rain::ServerManager smtpSM;
		smtpSM.setEventHandlers(SMTPServer::onConnect, SMTPServer::onMessage, SMTPServer::onDisconnect, &smtpCCP);
		smtpSM.setRecvBufLen(Rain::strToT<std::size_t>(config["smtp-transfer-buffer"]));
		if (!smtpSM.setServerListen(25, 25)) {
			Rain::tsCout("SMTP server listening on port ", smtpSM.getListeningPort(), ".\r\n");
		} else {
			Rain::tsCout("Fatal error: could not setup SMTP server listening.\r\n");
			fflush(stdout);
			DWORD error = GetLastError();
			Rain::reportError(error, "Fatal error: could not setup SMTP server listening.");
			WSACleanup();
			if (hFMemLeak != NULL)
				CloseHandle(hFMemLeak);
			return error;
		}

		//process commands
		while (true) {
			static std::string command, tmp;
			Rain::tsCout("Accepting commands...\r\n");
			std::cin >> command;
			Rain::tsCout("Command: " + command + "\r\n");
			std::getline(std::cin, tmp);

			auto handler = commandHandlers.find(command);
			if (handler != commandHandlers.end()) {
				if (handler->second(cmhParam) != 0)
					break;
			} else {
				Rain::tsCout("Command not recognized.\r\n");
			}
		}

		Rain::tsCout("The program has terminated.\r\n");
		fflush(stdout);

		logger.setStdHandleSrc(STD_OUTPUT_HANDLE, false);

		return 0;
	}
}  // namespace Emilia