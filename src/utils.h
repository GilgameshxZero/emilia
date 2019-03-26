#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "constants.h"
#include "main-param.h"
#include "http-server.h"
#include "smtp-server.h"
#include "deploy-server.h"

#include <ShellAPI.h>
#pragma comment(lib, "Version.lib")

namespace Emilia {
	//initialize a default project at an existing directory
	void initProjectDir(std::string dir);

	//initialize MainParam given a project directory
	void initMainParam(std::string project, MainParam &mp);

	//destroy MainParam and free all parameters
	void freeMainParam(MainParam &mp);

	//start servers with an int parameter
	void startServers(MainParam &mp, unsigned long long which);

	//restart process, keeping the same project and server context
	void prepRestart(std::string project, Rain::ServerManager *httpSM, Rain::ServerManager *smtpSM, std::string copySrc = "");

	//return a MAJOR.MINOR.REVISION.BUILD version string
	std::string getVersionStr();

	//inject "exit\r" into the console, prompting the program to exit peacefully
	void injectExitCommand();
}