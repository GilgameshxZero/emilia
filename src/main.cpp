#include "main.h"

int main(int argc, char *argv[]) {
	//restart the application if it didn't finish successfully
	//NOTE: only restarts if the application has been running successfully for 1 minute already, as per the spec in RegisterApplicationRestart
	DWORD apiReturn = RegisterApplicationRestart(Rain::mbStrToWStr("-r " + Rain::tToStr(static_cast<int>(Emilia::CMD_OPTION_R::CRASH))).c_str(), 0);
	if (apiReturn != S_OK) {
		Rain::reportError(apiReturn, "RegisterApplicationRestart failed; continuing...");
	}

	//set the ctrl+c handler
	if (!SetConsoleCtrlHandler([](DWORD fdwCtrlType) -> BOOL {
		switch (fdwCtrlType) {
		case CTRL_C_EVENT:
			Rain::tsCout("CTRL+C signal detected. Exiting...", Rain::CRLF);
			std::cout.flush();
			Emilia::injectExitCommand();
			return TRUE;
		default:
			return FALSE;
		}
	}, TRUE)) {
		Rain::reportError(GetLastError(), "SetConsoleCtrlHandler failed while capturing ctrl+c signal; continuing...");
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

		//by default, start both the HTTP and SMTP server
		unsigned long long initialServers = 0b11;

		//the first argument is always the executable, so don't process it
		if (argc >= 2) {
			for (int a = 1; a < argc; a++) {
				//in the format -<string>; this trims the initial -
				std::string arg = Rain::strTrimWhite(argv[a]).substr(1);
				typedef int (*cmdArgHandler)(
					const int, //argc
					int*, //&a
					char* [], //argv
					std::string*, //&project
					unsigned long long* //&initialServers
				);
				std::map<std::string, cmdArgHandler> cmdArgHandlers;

				//the following handlers return nonzero to abort the main program
				cmdArgHandlers["h"] = cmdArgHandlers["help"] = [](const int argc, int* a, char* argv[], std::string* project, unsigned long long* initialServers) -> int {
					Rain::tsCout("Please read the readme for further instructions.", Rain::CRLF);
					return 1;
				};
				cmdArgHandlers["r"] = cmdArgHandlers["restart"] = [](const int argc, int* a, char* argv[], std::string* project, unsigned long long* initialServers) -> int {
					CMD_OPTION_R rCode;
					if (*a == argc - 1) {
						rCode = CMD_OPTION_R::NONE;
					} else {
						rCode = static_cast<CMD_OPTION_R>(Rain::strToT<int>(argv[++ * a]));
					}
					switch (rCode) {
					case CMD_OPTION_R::CRASH:
						Rain::tsCout("Restarting from crash with RegisterApplicationRestart...", Rain::CRLF);
						break;
					case CMD_OPTION_R::DEPLOY:
						Rain::tsCout("Restarting from planned restart during sync...", Rain::CRLF);
						break;
					default:
						Rain::reportError(0, "Unrecognized restart (-r) code: " + Rain::tToStr(static_cast<int>(rCode)) + "; continuing...");
					}
					return 0;
				};
				cmdArgHandlers["p"] = cmdArgHandlers["project"] = [](const int argc, int* a, char* argv[], std::string* project, unsigned long long* initialServers) -> int {
					if (*a == argc - 1) {
						Rain::reportError(0, "Unrecognized project (-p) option; continuing...");
						*project = "";
					} else {
						//convert the project path to a directory if not already
						*project = Rain::standardizeDirPath(Rain::pathToAbsolute(Rain::strTrimWhite(argv[++ * a])));
					}
					return 0;
				};
				cmdArgHandlers["s"] = cmdArgHandlers["servers"] = [](const int argc, int* a, char* argv[], std::string* project, unsigned long long* initialServers) -> int {
					if (*a == argc - 1) {
						Rain::reportError(0, "Unrecognized server (-s) option; continuing...");
						*initialServers = 0;
					} else {
						*initialServers = Rain::strToT<unsigned long long>(argv[++ * a]);
					}
					return 0;
				};

				//call the handler
				std::map<std::string, cmdArgHandler>::iterator handler = cmdArgHandlers.find(arg);
				if (handler == cmdArgHandlers.end()) {
					Rain::reportError(0, "Unrecognized command line options; continuing...");
				} else {
					if (handler->second(argc, &a, argv, &project, &initialServers)) {
						return 0;
					}
				}
			}
		}

		//determine the project to equip
		if (project.size() == 0) {
			Rain::tsCout("Searching for nearby projects (max ", MAX_PROJECT_DIR_SEARCH, " directories)...", Rain::CRLF);
			std::cout.flush();

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
				std::cout.flush();
				project = Rain::pathToAbsolute(Rain::getPathDir(Rain::getExePath()));
				initProjectDir(project);
			}
		}
		Rain::tsCout("Project: ", project, Rain::CRLF);
		std::cout.flush();

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
		std::string command, tmp;
		while (true) {
			Rain::tsCout("Accepting commands...", Rain::CRLF);
			std::cout.flush();

			std::cin >> command;
			if (std::cin.fail()) {
				Rain::reportError(0, "std::cin in Emilia::start failed; aborting...");
				break;
			}
			Rain::strTrimWhite(&command);

			//remove the newline from cin after getting the command
			std::getline(std::cin, tmp);

			//delegate to correct handler
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