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
	Rain::HttpSocket client(Rain::Socket(), 16);
	std::cout << "socket.connect: " << client.connect("mit.edu", "80")
						<< std::endl;
	std::cout << "socket.send: "
						<< client.send("GET / HTTP/1.1\r\nHost: web.mit.edu\r\n\r\n")
						<< std::endl;
	int headerLen = client.recvHeader();
	std::cout << "socket.recvHeader: " << headerLen << std::endl;
	char buffer[8192];
	memset(buffer, 0, sizeof(buffer));
	std::cout << "socket.recv: " << client.recv(buffer, 8192) << std::endl;
	std::cout << buffer << std::endl;
	std::cout << "socket.close: " << client.close() << std::endl;
	/*
	Rain::Socket server;
	std::cout << "socket.bind: " << server.bind("80") << std::endl;
	std::cout << "socket.listen: " << server.listen() << std::endl;
	Rain::Socket conn(server.accept());
	memset(buffer, 0, sizeof(buffer));
	std::cout << "socket.recv: " << conn.recv(buffer, 8192) << std::endl;
	std::cout << buffer << std::endl;

	server.close();
	conn.close();*/
	Rain::Socket::cleanup();

	return 0;
}
