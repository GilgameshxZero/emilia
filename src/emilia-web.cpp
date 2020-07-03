#include "emilia-web.hpp"

int main(int argc, char *argv[], char **envp) {
	long long httpPort = 80, httpsPort = 443;
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

	std::cout << std::endl << "Starting socket operations..." << std::endl;
	;
	Rain::Socket socket;
	std::cout << "socket.create: " << socket.create() << std::endl;
	std::cout << "socket.connect: " << socket.connect("usaco.org", "80")
						<< std::endl;
	std::cout << "socket.send: "
						<< socket.send("GET / HTTP/1.1\r\nHost: usaco.org\r\n\r\n")
						<< std::endl;
	char buffer[8192];
	memset(buffer, 0, sizeof(buffer));
	std::cout << "socket.recv: " << socket.recv(buffer, 8192) << std::endl;
	std::cout << buffer << std::endl;
	std::cout << "socket.close: " << socket.close() << std::endl;
	std::cout << "Socket shutdown." << std::endl;

	return 0;
}
