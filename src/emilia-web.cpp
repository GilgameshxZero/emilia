#include "emilia-web.hpp"

#include <iostream>

int main(int argc, char *argv[], char **envp) {
	Rain::HttpServer server;
	server.onNew = [](Rain::HttpServerSlave *socket) {
		std::cout << "onNew" << std::endl;
	};
	server.onClose = [](Rain::HttpServerSlave *socket) {
		std::cout << "onClose" << std::endl;
	};
	std::cout << "Starting server: " << server.serve(false) << std::endl;
	Rain::sleep(10000);
	std::cout << "Stopping server: " << server.close() << std::endl;

	Rain::Socket::cleanup();
	return 0;
}
