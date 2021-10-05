#include <emilia.hpp>

#include <http.hpp>
#include <smtp.hpp>
#include <state.hpp>

#include <rain.hpp>

int main(int argc, char const *argv[]) {
	std::stringstream signature;
	signature << "emilia " << EMILIA_VERSION_MAJOR << "." << EMILIA_VERSION_MINOR
						<< "." << EMILIA_VERSION_REVISION << "." << EMILIA_VERSION_BUILD
						<< " / rain " << RAIN_VERSION_MAJOR << "." << RAIN_VERSION_MINOR
						<< "." << RAIN_VERSION_REVISION << "." << RAIN_VERSION_BUILD;
	std::cout << signature.str() << std::endl;
	std::srand(static_cast<unsigned int>(time(nullptr)));

	// Parse command line options.
	Emilia::State state;
	state.host = "gilgamesh.cc";
	std::string httpPort = "0", smtpPort = "0", smtpForwardStr = "";
	bool showHelp = false;

	Rain::String::CommandLineParser parser;
	parser.addParser("help", showHelp);
	parser.addParser("h", showHelp);
	parser.addParser("domain", state.host.node);
	parser.addParser("http-port", httpPort);
	parser.addParser("smtp-port", smtpPort);
	parser.addParser("smtp-forward", smtpForwardStr);
	parser.addParser("smtp-password", state.smtpPassword);
	try {
		parser.parse(argc - 1, argv + 1);
	} catch (...) {
		std::cout
			<< "Failed to parse command-line options. Consider running with --help."
			<< std::endl;
		return -1;
	}
	state.smtpForward = smtpForwardStr;

	// Show help if prompted.
	if (showHelp) {
		std::cout
			<< "Command-line options (default specified in parenthesis):\n"
			<< "--help, -h (off): Display this help message and exit.\n"
			<< "--http-port (\"0\"): Alter the listening port of the HTTP server.\n"
			<< "--smtp-port (\"0\"): Alter the listening port of the SMTP server.\n"
			<< "--smtp-forward (\"\"): If specified, forwards all received emails to "
				 "listed address.\n"
			<< "--domain (\"gilgamesh.cc\"): Configured domain for SMTP server and "
				 "HTTP endpoints.\n"
			<< "--smtp-password (\"\"): If specified, enables authenticated "
				 "clients to send from domain.\n"
			<< std::endl;
		return 0;
	}

	state.signature = signature.str();
	std::time_t time =
		std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	Rain::Time::localtime_r(&time, &state.processBegin);

	// Launch both servers.
	state.httpServer.reset(new Emilia::Http::Server(state, {"", httpPort}));
	std::cout << "HTTP server listening on " << state.httpServer->host() << "..."
						<< std::endl;
	state.smtpServer.reset(new Emilia::Smtp::Server(state, {"", smtpPort}));
	std::cout << "SMTP server listening on " << state.smtpServer->host() << "..."
						<< std::endl;

	// Parse commands.
	std::cout << "Listening to commands..." << std::endl;
	std::string command;
	while (true) {
		std::getline(std::cin, command);
		Rain::String::toLower(Rain::String::trimWhitespace(command));
		if (command == "help") {
			std::cout
				<< "Command options:\n"
				<< "help: Display this help message.\n"
				<< "exit: Attempt graceful shutdown and exit.\n"
				<< "stop http: Graceful shutdown HTTP server.\n"
				<< "stop smtp: Graceful shutdown SMTP server.\n"
				<< "start http: If HTTP server stopped, start it.\n"
				<< "start smtp: If SMTP server stopped, start it.\n"
				<< "modify http: Modify HTTP server port. Restart required.\n"
				<< "modify smtp: Modify SMTP server port. Restart required.\n"
				<< "clear cache: Clear HTTP static file cache.\n"
				<< "clear outbox: Clear non-PENDING outbox history.\n"
				<< "modify forward: Modify SMTP forwarding address.\n"
				<< "modify password: Modify SMTP authentication password.\n"
				<< "toggle echo: Toggles echoing of all parsed requests/responses to "
					 "the command-line.\n"
				<< std::endl;
		} else if (command == "exit") {
			break;
		} else if (command == "stop http") {
			state.httpServer.reset();
			std::cout << "Stopped." << std::endl;
		} else if (command == "stop smtp") {
			state.smtpServer.reset();
			std::cout << "Stopped." << std::endl;
		} else if (command == "start http") {
			if (state.httpServer) {
				std::cout << "HTTP server already listening on "
									<< state.httpServer->host() << '.' << std::endl;
			} else {
				Rain::Error::consumeThrowable(
					[&state, &httpPort]() {
						state.httpServer.reset(
							new Emilia::Http::Server(state, {"", httpPort}));
						std::cout << "HTTP server listening on " << state.httpServer->host()
											<< "..." << std::endl;
					},
					RAIN_ERROR_LOCATION)();
			}
		} else if (command == "start smtp") {
			if (state.smtpServer) {
				std::cout << "SMTP server already listening on "
									<< state.smtpServer->host() << '.' << std::endl;
			} else {
				Rain::Error::consumeThrowable(
					[&state, &smtpPort]() {
						state.smtpServer.reset(
							new Emilia::Smtp::Server(state, {"", smtpPort}));
						std::cout << "SMTP server listening on " << state.smtpServer->host()
											<< "..." << std::endl;
					},
					RAIN_ERROR_LOCATION)();
			}
		} else if (command == "modify http") {
			std::cout << "Modified HTTP port: ";
			std::getline(std::cin, httpPort);
			std::cout << "Modified." << std::endl;
		} else if (command == "modify smtp") {
			std::cout << "Modified SMTP port: ";
			std::getline(std::cin, smtpPort);
			std::cout << "Modified." << std::endl;
		} else if (command == "clear cache") {
			std::unique_lock lck(state.fileCacheMtx);
			state.fileCache.clear();
			std::cout << "Cleared." << std::endl;
		} else if (command == "clear outbox") {
			std::unique_lock lck(state.outboxMtx);
			std::cout << state.outbox.size() << " total in outbox." << std::endl;
			for (auto it = state.outbox.begin(); it != state.outbox.end();) {
				if (it->status == Emilia::Envelope::Status::PENDING) {
					it++;
					continue;
				}
				it = state.outbox.erase(it);
			}
			std::cout << state.outbox.size() << " remaining in outbox." << std::endl;
		} else if (command == "modify forward") {
			std::cout << "Modified SMTP forward: ";
			std::getline(std::cin, smtpForwardStr);
			state.smtpForward = smtpForwardStr;
			std::cout << "Modified." << std::endl;
		} else if (command == "modify password") {
			std::cout << "Modified SMTP password: ";
			std::getline(std::cin, state.smtpPassword);
			std::cout << "Modified." << std::endl;
		} else if (command == "toggle echo") {
			state.echo = !state.echo;
			std::cout << "Echo is " << (state.echo ? "on" : "off") << '.'
								<< std::endl;
		} else {
			std::cout << "Invalid command: " << command << '.' << std::endl;
		}
	}

	// Attempt graceful close.
	std::cout << "Attempting graceful close of HTTP and SMTP servers..."
						<< std::endl;
	state.httpServer.reset();
	state.smtpServer.reset();
	std::cout << "Gracefully closed HTTP and SMTP servers." << std::endl;

	return 0;
}
