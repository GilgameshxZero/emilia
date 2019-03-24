#include "main.h"

int main(int argc, char *argv[]) {
	//try to create a named mutex; if it already exists, then another instance of this application is running so terminate this
	//mutex name cannot have backslashes
	std::string mutexName = Rain::pathToAbsolute(Rain::getExePath());
	std::replace(mutexName.begin(), mutexName.end(), '\\', '/');
	CreateMutex(NULL, FALSE, mutexName.c_str());
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		std::cout << "Another instance of process already running." << Rain::CRLF;
		return -1;
	}

	//restart the application if it didn't finish successfully
	if (FAILED(RegisterApplicationRestart(Rain::mbStrToWStr("-r " + Emilia::CMDL_CRASH_RESTART + "-s 3").c_str(), 0))) {
		std::cout << "RegisterApplicationRestart failed." << Rain::CRLF;
	}

	int error = Emilia::start(argc, argv);
	std::cout << "Start returned " << error << "." << Rain::CRLF;

	//finished successfully, so don't restart it
	UnregisterApplicationRestart();

	return error;
}

namespace Emilia {
	int start(int argc, char *argv[]) {
		//version and process command line options
		std::cout << "Emilia " << getVersionStr() << Rain::CRLF << Rain::CRLF;

		//stores the project path
		std::string project;
		unsigned long long defaultServers = 0;
		
		//the first argument is always the executable
		if (argc >= 2) {
			for (int a = 1; a < argc; a++) {
				std::string arg = Rain::strTrimWhite(argv[a]);
				if (arg == "-r") {
					std::string code = Rain::strTrimWhite(argv[++a]);
					if (code == CMDL_DEPLOY_RESTART) {
						std::cout << "Emilia restarted after deployment sync." << Rain::CRLF;
					} else if (code == CMDL_CRASH_RESTART) {
						std::cout << "Emilia restarted after crash." << Rain::CRLF;
					}
				} else if (arg == "-p") {
					project = Rain::pathToAbsolute(Rain::strTrimWhite(argv[++a]));
				} else if (arg == "-s") {
					defaultServers = Rain::strToT<unsigned long long>(argv[++a]);
				} else {
					std::cout << "Command line option not recognized: " << argv[a] << Rain::CRLF;
				}
			}
		}

		//determine the project to equip
		if (project.size() == 0) {
			int limit = 1000;
			std::cout << "Searching for nearby projects (max " << limit << ")..." << Rain::CRLF;

			std::queue<std::string> next;
			std::set<std::string> searched;
			next.push(Rain::pathToAbsolute(Rain::getPathDir(Rain::getExePath())));
			while (!next.empty() && project.size() == 0 && limit-- > 0) {
				std::string cur = next.front();
				searched.insert(cur);
				next.pop();

				std::vector<std::string> dirs = Rain::getDirs(cur);
				for (std::size_t a = 0; a < dirs.size(); a++) {
					if (dirs[a] == ".emilia") {
						project = cur;
						break;
					} else {
						std::string path = Rain::standardizeDirPath(Rain::pathToAbsolute(cur + dirs[a]));
						if (searched.find(path) == searched.end()) {
							next.push(path);
						}
					}
				}
			}

			if (project.size() == 0) {
				std::cout << "Could not find any nearby projects. Creating a project at the current directory." << Rain::CRLF;
				project = Rain::pathToAbsolute(Rain::getPathDir(Rain::getExePath()));
				initProjectDir(project);
			}
		}

		std::cout << "Project: " << project << Rain::CRLF;

		MainParam mp;
		initMainParam(project, mp);
		startServers(mp, defaultServers);

		//wait for commands
		std::map<std::string, CommandHandler::CommandHandler> commandHandlers{
			{"exit", CommandHandler::Exit},
			{"restart", CommandHandler::Restart},
			{"server", CommandHandler::Server},
			{"connect", CommandHandler::Connect},
			{"disconnect", CommandHandler::Disconnect},
			{"project", CommandHandler::Project},
			{"sync", CommandHandler::Sync}
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
				if (handler->second(mp) != 0) {
					break;
				}
			} else {
				Rain::tsCout("Command not recognized.", Rain::CRLF);
			}
		}

		//cleaning up
		freeMainParam(mp);

		return 0;
	}
}