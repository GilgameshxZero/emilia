#include "main.h"

int main(int argc, char *argv[]) {
	//restart the application if it didn't finish successfully
	//NOTE: only restarts if the application has been running successfully for 1 minute already
	DWORD apiReturn = RegisterApplicationRestart(Rain::mbStrToWStr("-r " + Rain::tToStr(static_cast<int>(Emilia::CMD_OPTION_R::CRASH)) + " -s 3").c_str(), 0);
	if (apiReturn != S_OK) {
		Rain::reportError(apiReturn, "RegisterApplicationRestart failed; continuing...");
	}

	//set the ctrl+c handler
	if (!SetConsoleCtrlHandler([](DWORD fdwCtrlType) ->BOOL {
		switch (fdwCtrlType) {
		case CTRL_C_EVENT:
			Rain::tsCout("CTRL+C signal detected. Exiting...", Rain::CRLF);
			Emilia::injectExitCommand();
			return TRUE;
		default:
			return FALSE;
		}
	}, TRUE)) {
		Rain::reportError(GetLastError(), "SetConsoleCtrlHandler failed; continuing...");
	}

	//run the application
	int startError = Emilia::start(argc, argv);
	if (startError != 0) {
		Rain::reportError(startError, "Emilia::start returned abnormal error code; aborting...");
		return startError;
	}

	//finished successfully, so don't restart it
	apiReturn = UnregisterApplicationRestart();
	if (apiReturn != S_OK) {
		Rain::reportError(apiReturn, "UnregisterApplicationRestart failed; aborting...");
		return apiReturn;
	}
	return 0;
}

namespace Emilia {
	int start(int argc, char *argv[]) {
		Rain::tsCout("Emilia ", getVersionStr(), Rain::CRLF, Rain::CRLF);

		//process command line options
		std::string project;
		unsigned long long initialServers = 0;

		//the first argument is always the executable
		if (argc >= 2) {
			for (int a = 1; a < argc; a++) {
				//in the format -<x>
				std::string arg = Rain::strTrimWhite(argv[a]);
				CMD_OPTION_R rCode;
				switch (arg[1]) {
				case 'h':
					Rain::tsCout("Please read the readme for further instructions.", Rain::CRLF);
					return 0;
				case 'r':
					if (a == argc - 1) {
						rCode = CMD_OPTION_R::NONE;
					} else {
						rCode = static_cast<CMD_OPTION_R>(Rain::strToT<int>(argv[++a]));
					}
					switch (rCode) {
					case CMD_OPTION_R::CRASH:
						Rain::tsCout("Restarting from crash with RegisterApplicationRestart...", Rain::CRLF);
						break;
					case CMD_OPTION_R::DEPLOY:
						Rain::tsCout("Restarting from planned restart during sync...", Rain::CRLF);
						break;
					default:
						Rain::reportError(0, "Unrecognized -r code: " + Rain::tToStr(static_cast<int>(rCode)) + "; continuing...");
					}
					break;
				case 'p':
					if (a == argc - 1) {
						Rain::reportError(0, "Unrecognized -p option; continuing...");
						project = "";
					} else {
						project = Rain::pathToAbsolute(Rain::strTrimWhite(argv[++a]));
					}
					break;
				case 's':
					if (a == argc - 1) {
						Rain::reportError(0, "Unrecognized -s option; continuing...");
						initialServers = 0;
					} else {
						initialServers = Rain::strToT<unsigned long long>(argv[++a]);
					}
					break;
				default:
					Rain::reportError(0, "Unrecognized command line option: " + arg + "; continuing...");
				}
			}
		}

		//determine the project to equip
		if (project.size() == 0) {
			Rain::tsCout("Searching for nearby projects (max ", MAX_PROJECT_DIR_SEARCH, " directories)...", Rain::CRLF);

			std::queue<std::string> next;
			std::set<std::string> searched;
			next.push(Rain::pathToAbsolute(Rain::getPathDir(Rain::getExePath())));
			for (int a = 0; a < MAX_PROJECT_DIR_SEARCH && !next.empty(); a++) {
				std::string cur = next.front();
				searched.insert(cur);
				next.pop();

				std::vector<std::string> dirs = Rain::getDirs(cur);
				for (std::size_t b = 0; b < dirs.size(); b++) {
					if (dirs[b] + '\\' == PROJECT_DIR) {
						project = cur;
						break;
					} else {
						std::string path = Rain::standardizeDirPath(Rain::pathToAbsolute(cur + dirs[b]));
						if (searched.find(path) == searched.end()) {
							next.push(path);
						}
					}
				}
			}

			if (project.size() == 0) {
				Rain::tsCout("Could not find any nearby projects. Creating a project at the current directory...", Rain::CRLF);
				project = Rain::pathToAbsolute(Rain::getPathDir(Rain::getExePath()));
				initProjectDir(project);
			}
		}
		Rain::tsCout("Project: ", project, Rain::CRLF);

		MainParam mp;
		initMainParam(project, mp);
		startServers(mp, initialServers);

		//wait for commands
		std::map<std::string, CommandHandler::CommandHandler> commandHandlers{
			{"exit", CommandHandler::exit},
			{"restart", CommandHandler::restart},
			{"server", CommandHandler::server},
			{"connect", CommandHandler::connect},
			{"disconnect", CommandHandler::disconnect},
			{"project", CommandHandler::project},
			{"sync", CommandHandler::sync}
		};
		while (true) {
			std::string command, tmp;
			Rain::tsCout("Accepting commands...", Rain::CRLF);
			std::cout.flush();

			std::cin >> command;
			if (std::cin.fail()) {
				Rain::reportError(0, "std::cin in Emilia::start failed; aborting...");
				break;
			}
			Rain::strTrimWhite(command);
			std::getline(std::cin, tmp);

			auto handler = commandHandlers.find(command);
			if (handler != commandHandlers.end()) {
				if (handler->second(mp) != 0) {
					break;
				}
			} else {
				Rain::reportError(0, "Unable to parse user command: " + command + "; continuing...");
			}
		}

		//cleaning up
		freeMainParam(mp);
		return 0;
	}
}