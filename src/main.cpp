#include "main.h"

int main(int argc, char* argv[]) {
    int error;

    //if there's an exception, just restart the servers
    while (true) {
        try {
            error = Emilia::start(argc, argv);
            break;
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
            std::cout << "Retrying..." << std::endl;
        }
    }

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
    HTTPServer::ConnectionCallerParam smtpCCP;
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

    //update server setup
    DWORD updateServerPort = Rain::strToT<DWORD>(config["update-server-port"]);

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