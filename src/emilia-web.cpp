#include "emilia-web.hpp"

#include <iostream>

int main(int argc, char *argv[], char **envp) {
	Rain::HttpServer server;
	server.onRequest = [](Rain::HttpRequest *req) {
		std::cout << req->method << " " << req->uri << " HTTP/" << req->version
							<< std::endl;
		for (auto it = req->headers.begin(); it != req->headers.end(); it++) {
			std::cout << it->first << ": " << it->second << std::endl;
		}
	};
	std::cout << "Starting server: " << server.serve(false) << std::endl;
	Rain::sleep(60000);
	std::cout << "Stopping server: " << server.close() << std::endl;

	Rain::Socket::cleanup();
	return 0;
}
