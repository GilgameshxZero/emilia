#include "main.h"

int main(int argc, char* argv[]) {
    int error = Emilia::start(argc, argv);
    std::cout << "start returned error code " << error << ".\r\n";
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

    //server setups
    /*DWORD updateServerPort = Rain::strToT<DWORD>(config["update-server-port"]);

		std::map<DWORD, std::pair<ConnectionCallerParam, Rain::ServerManager>> servers;
		servers.insert(std::make_pair(80, std::make_pair(ConnectionCallerParam(), Rain::ServerManager())));
		servers.insert(std::make_pair(25, std::make_pair(ConnectionCallerParam(), Rain::ServerManager())));
		servers.insert(std::make_pair(updateServerPort, std::make_pair(ConnectionCallerParam(), Rain::ServerManager())));*/

    //http server setup
    HTTPServer::ConnectionCallerParam ccParam;
    ccParam.config = &config;
    ccParam.logger = &logger;
    ccParam.connectedClients = 0;

    Rain::ServerManager sm;
    sm.setEventHandlers(HTTPServer::onConnect, HTTPServer::onMessage, HTTPServer::onDisconnect, &ccParam);
    sm.setRecvBufLen(Rain::strToT<std::size_t>(config["http-transfer-buffer"]));
    if (!sm.setServerListen(80, 80)) {
        Rain::tsCout("Server listening on port ", sm.getListeningPort(), ".\r\n");
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

    //process commands
    static std::map<std::string, CommandMethodHandler> commandHandlers{
        {"exit", CHExit},
        {"help", CHHelp},
    };
    CommandHandlerParam cmhParam;
    cmhParam.config = &config;
    cmhParam.logger = &logger;
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

    logger.setStdHandleSrc(STD_OUTPUT_HANDLE, false);

    Rain::tsCout("The program has terminated.\r\n");
    fflush(stdout);
    return 0;
}
}  // namespace Emilia