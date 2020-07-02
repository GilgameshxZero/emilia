#include "emilia-api.hpp"

int main(int argc, char *argv[], char **envp) {
	long long httpPort = 80,
						httpsPort = 443;
	char rootDir[1024] = "./";
	bool autoHeaders = false;

	Rain::CommandLineParser parser;
	parser.addParser("http-port", &httpPort);
	parser.addParser("https-port", &httpsPort);
	parser.addParser("root-dir", rootDir, 1024);
	parser.addParser("D", rootDir, 1024);
	parser.addParser("auto-headers", &autoHeaders);
	parser.parse(argc - 1, argv + 1);

	std::cout << "Starting HTTP server on port " << httpPort << std::endl
						<< "Starting HTTPS server on port " << httpsPort << std::endl
						<< "Serving directory " << rootDir << std::endl
						<< "Auto headers is " << (autoHeaders ? "active" : "inactive")
						<< "." << std::endl;

	return 0;
}
