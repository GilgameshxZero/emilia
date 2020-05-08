#pragma once
#include "../rain/src/rain.hpp"

namespace Emilia {
	namespace DeployServer {
		typedef int(*RequestMethodHandler)(Rain::ServerSocketManager::DelegateHandlerParam &);

		struct SyncInfo {
			std::string stage = "send-emilia";
			time_t remoteEmiliaTime;
			std::string remoteEmiliaBinary;
			time_t remoteIndexTime;
			std::string remoteIndex;
			Rain::Configuration *index;
			std::map<std::string, std::pair<bool, time_t>> status, remoteStatus;
			std::thread sendThread;

			std::vector<std::pair<std::string, size_t>> receiving, sending;
			int curFile;
			size_t curBytes;
		};

		struct ConnectionCallerParam {
			std::string project;
			Rain::Configuration *config = NULL;
			Rain::LogStream *logDeploy = NULL;
			Rain::ServerManager *httpSM = NULL,
				*smtpSM = NULL;

			//the current SSM of connected client; only one client can connect at a time
			Rain::ServerSocketManager *ssm = NULL;
		};

		struct ConnectionDelegateParam {
			SyncInfo si;

			//parsed first section of the request
			std::string requestMethod;

			//whether current socket is authenticated
			bool authenticated;
		};
	}
}