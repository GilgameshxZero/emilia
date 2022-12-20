#include <emilia.hpp>

#include <http.hpp>
#include <smtp.hpp>

#include <rain.hpp>

int main(int argc, char const *argv[]) {
	return Rain::Error::consumeThrowable(
		[argc, argv]() {
			std::srand(static_cast<unsigned int>(time(nullptr)));
			std::cout << "emilia " << EMILIA_VERSION_MAJOR << "."
								<< EMILIA_VERSION_MINOR << "." << EMILIA_VERSION_REVISION << "."
								<< EMILIA_VERSION_BUILD << " / rain " << RAIN_VERSION_MAJOR
								<< "." << RAIN_VERSION_MINOR << "." << RAIN_VERSION_REVISION
								<< "." << RAIN_VERSION_BUILD << std::endl;

			// Parse command line options.
			std::string httpPort{"0"}, smtpPort{"0"}, httpPassword, smtpForwardStr,
				smtpPassword;
			bool showHelp{false};

			Rain::String::CommandLineParser parser;
			parser.addParser("help", showHelp);
			parser.addParser("h", showHelp);
			parser.addParser("http-port", httpPort);
			parser.addParser("smtp-port", smtpPort);
			parser.addParser("http-password", httpPassword);
			parser.addParser("smtp-forward", smtpForwardStr);
			parser.addParser("smtp-password", smtpPassword);
			try {
				parser.parse(argc - 1, argv + 1);
			} catch (...) {
				std::cout << "Failed to parse command-line options. Consider running "
										 "with --help."
									<< std::endl;
				return -1;
			}
			if (showHelp) {
				// Show help if prompted.
				std::cout
					<< "Command-line options (default specified in parenthesis):\n"
					<< "--help, -h (off): Display this help message and exit.\n"
					<< "--http-port (\"0\"): Alter the listening port of the HTTP "
						 "server.\n"
					<< "--smtp-port (\"0\"): Alter the listening port of the SMTP "
						 "server.\n"
					<< "--http-password (\"\"): Password for authenticated routes.\n"
					<< "--smtp-forward (\"\"): If specified, forwards all received "
						 "emails to "
						 "listed address.\n"
					<< "--smtp-password (\"\"): If specified, enables authenticated "
						 "clients to send from domain.\n"
					<< std::endl;
				return 0;
			}

			// Shared state.
			Rain::Networking::Smtp::Mailbox smtpForward(smtpForwardStr);
			std::atomic_bool echo{false};
			std::time_t time{
				std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
			std::tm processBegin;
			Rain::Time::localtime_r(&time, &processBegin);

			// Launch both servers. HTTP server requires SMTP server state.
			auto newSmtpServer = [&]() {
				return new Emilia::Smtp::Server(
					{"", smtpPort}, echo, smtpForward, smtpPassword);
			};
			std::unique_ptr<Emilia::Smtp::Server> smtpServer(newSmtpServer());
			std::cout << "SMTP server listening on " << smtpServer->host() << "..."
								<< std::endl;
			auto newHttpServer = [&]() {
				return new Emilia::Http::Server(
					{"", httpPort}, httpPassword, processBegin, smtpServer, echo);
			};
			std::unique_ptr<Emilia::Http::Server> httpServer(newHttpServer());
			std::cout << "HTTP server listening on " << httpServer->host() << "..."
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
						<< "stop smtp: Graceful shutdown SMTP server.\n"
						<< "stop http: Graceful shutdown HTTP server.\n"
						<< "start smtp: If SMTP server stopped, start it.\n"
						<< "start http: If HTTP server stopped, start it.\n"
						<< "modify smtp: Modify SMTP server port. Restart required.\n"
						<< "modify http: Modify HTTP server port. Restart required.\n"
						<< "clear outbox: Clear non-PENDING outbox history.\n"
						<< "trigger outbox: Trigger a send of any ready PENDING "
							 "envelopes.\n"
						<< "modify forward: Modify SMTP forwarding address.\n"
						<< "modify password: Modify SMTP authentication password.\n"
						<< "toggle echo: Toggles echoing of all parsed requests/responses "
							 "to "
							 "the command-line.\n"
						<< "refresh: Refresh HTTP snapshots tags.\n"
						<< std::endl;
				} else if (command == "exit") {
					break;
				} else if (command == "stop smtp") {
					smtpServer.reset();
					std::cout << "Stopped." << std::endl;
				} else if (command == "stop http") {
					httpServer.reset();
					std::cout << "Stopped." << std::endl;
				} else if (command == "start smtp") {
					if (smtpServer) {
						std::cout << "SMTP server already listening on "
											<< smtpServer->host() << '.' << std::endl;
					} else {
						Rain::Error::consumeThrowable(
							[&]() {
								smtpServer.reset(newSmtpServer());
								std::cout << "SMTP server listening on " << smtpServer->host()
													<< "..." << std::endl;
							},
							RAIN_ERROR_LOCATION)();
					}
				} else if (command == "start http") {
					if (httpServer) {
						std::cout << "HTTP server already listening on "
											<< httpServer->host() << '.' << std::endl;
					} else {
						Rain::Error::consumeThrowable(
							[&]() {
								httpServer.reset(newHttpServer());
								std::cout << "HTTP server listening on " << httpServer->host()
													<< "..." << std::endl;
							},
							RAIN_ERROR_LOCATION)();
					}
				} else if (command == "modify smtp") {
					std::cout << "Modified SMTP port: ";
					std::getline(std::cin, smtpPort);
					std::cout << "Modified." << std::endl;
				} else if (command == "modify http") {
					std::cout << "Modified HTTP port: ";
					std::getline(std::cin, httpPort);
					std::cout << "Modified." << std::endl;
				} else if (command == "clear outbox") {
					std::unique_lock lck(smtpServer->outboxMtx);
					std::cout << smtpServer->outbox.size() << " total in outbox."
										<< std::endl;
					for (auto it = smtpServer->outbox.begin();
							 it != smtpServer->outbox.end();) {
						if (it->status == Emilia::Envelope::Status::PENDING) {
							it++;
							continue;
						}
						it = smtpServer->outbox.erase(it);
					}
					std::cout << smtpServer->outbox.size() << " remaining in outbox."
										<< std::endl;
				} else if (command == "trigger outbox") {
					smtpServer->outboxEv.notify_one();
					std::cout << "Triggered outbox send event." << std::endl;
				} else if (command == "modify forward") {
					std::cout << "Modified SMTP forward: ";
					std::getline(std::cin, smtpForwardStr);
					smtpForward = smtpForwardStr;
					std::cout << "Modified." << std::endl;
				} else if (command == "modify password") {
					std::cout << "Modified SMTP password: ";
					std::getline(std::cin, smtpPassword);
					std::cout << "Modified." << std::endl;
				} else if (command == "toggle echo") {
					echo = !echo;
					std::cout << "Echo is " << (echo ? "on" : "off") << '.' << std::endl;
				} else if (command == "refresh") {
					httpServer->refreshSnapshots();
				} else {
					std::cout << "Invalid command: " << command << '.' << std::endl;
				}
			}

			// Attempt graceful close.
			std::cout << "Attempting graceful close of HTTP and SMTP servers..."
								<< std::endl;
			httpServer.reset();
			smtpServer.reset();
			std::cout << "Gracefully closed HTTP and SMTP servers." << std::endl;

			return 0;
		},
		RAIN_ERROR_LOCATION)();
}
