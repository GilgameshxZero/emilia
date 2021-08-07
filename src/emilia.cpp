#include <emilia.hpp>

#include <rain.hpp>

#include <http.hpp>
#include <smtp.hpp>

int main(int argc, char const *const argv[]) {
	using namespace Rain::Literal;

	std::cout << "emilia " << EMILIA_VERSION_MAJOR << "." << EMILIA_VERSION_MINOR
						<< "." << EMILIA_VERSION_REVISION << "." << EMILIA_VERSION_BUILD
						<< " / rain " << RAIN_VERSION_MAJOR << "." << RAIN_VERSION_MINOR
						<< "." << RAIN_VERSION_REVISION << "." << RAIN_VERSION_BUILD
						<< std::endl;
	std::srand(static_cast<unsigned int>(time(nullptr)));

	// Parse command line options.
	bool showHelp = false;
	std::string httpPort = "0", smtpPort = "0", forwardTo = "",
							domain = "gilgamesh.cc", sendAsPassword = "";

	Rain::String::CommandLineParser parser;
	parser.addParser("help", showHelp);
	parser.addParser("h", showHelp);
	parser.addParser("http-port", httpPort);
	parser.addParser("smtp-port", smtpPort);
	parser.addParser("forward-to", forwardTo);
	parser.addParser("domain", domain);
	parser.addParser("send-as-password", sendAsPassword);
	try {
		parser.parse(argc - 1, argv + 1);
	} catch (...) {
		std::cout
			<< "Failed to parse command-line options. Consider running with --help."
			<< std::endl;
		return -1;
	}

	// Show help if prompted.
	if (showHelp) {
		std::cout
			<< "Command-line options (default specified in parenthesis):\n"
			<< "--help, -h (off): Display this help message and exit.\n"
			<< "--http-port (\"0\"): Alter the listening port of the HTTP server.\n"
			<< "--smtp-port (\"0\"): Alter the listening port of the SMTP server.\n"
			<< "--forward-to (\"\"): If specified, forwards all received emails to "
				 "listed address.\n"
			<< "--domain (\"gilgamesh.cc\"): Configured domain for SMTP server.\n"
			<< "--send-as-password (\"\"): If specified, enables authenticated "
				 "clients to send from domain.\n"
			<< std::endl;
		return 0;
	}

	// Launch both servers.
	Emilia::Http::Server httpServer(256);
	httpServer.serve({"", httpPort});
	std::cout << "HTTP server listening on " << httpServer.getTargetHost()
						<< "..." << std::endl;

	Emilia::Smtp::Server smtpServer(
		256,
		Rain::Networking::Specification::ProtocolFamily::INET,
		1_zu << 10,
		1_zu << 10,
		60s,
		60s,
		forwardTo,
		domain,
		sendAsPassword);
	smtpServer.serve({"", smtpPort});
	std::cout << "SMTP server listening on " << smtpServer.getTargetHost()
						<< "..." << std::endl;

	// Parse commands.
	std::cout << "Listening to commands..." << std::endl;
	std::string command;
	while (true) {
		std::cin >> command;
		if (command == "exit") {
			break;
		} else {
			std::cout << "Invalid command: " << command << "." << std::endl;
		}
	}

	// Attempt graceful close.
	std::cout << "Attempting graceful close of HTTP and SMTP servers..."
						<< std::endl;
	if (httpServer.close() || smtpServer.close()) {
		std::cout << "Failed to gracefully close. Aborting..." << std::endl;
	} else {
		std::cout << "Gracefully closed HTTP and SMTP servers." << std::endl;
	}

	return 0;
}
