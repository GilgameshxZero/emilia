#pragma once

#include "rain-aeternum/rain-libraries.h"

#include "constants.h"
#include "main-param.h"

#include "deploy-server.h"

#include <ShellAPI.h>

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
}