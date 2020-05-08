#pragma once
#include "../rain/src/rain.hpp"

#include "constants.hpp"
#include "utils.hpp"
#include "deploy-server-param.hpp"

#include <ShellAPI.h>
#include <sys/utime.h>
#include <thread>

namespace Emilia {
	namespace DeployServer {
		int onConnect(void *funcParam);
		int onMessage(void *funcParam);
		int onDisconnect(void *funcParam);

		//specific handlers for different messages
		int authenticate(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhp);
		int sync(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhp);
		int server(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhp);
		int restart(Rain::ServerSocketManager::DelegateHandlerParam &ssmdhp);

		//sync routine shared by client and server
		void syncRoutine(
			std::string project,
			Rain::Configuration &config,
			Rain::ServerManager *httpSM,
			Rain::ServerManager *smtpSM,
			Rain::LogStream &logDeploy,
			Rain::ConditionVariable *completeCV,
			std::string &message,
			Rain::SocketManager &remote,
			SyncInfo &si);

		//lists files under a project directory excluding ignored files and the index
		std::vector<std::string> listProjectFiles(std::string project, Rain::Configuration &ignored);

		//helper routine of sync which sends a list of files through a socket
		void syncSendFiles(std::string project, int bufferSize, std::vector<std::pair<std::string, size_t>> *sending, Rain::SocketManager *remote);
	}
}