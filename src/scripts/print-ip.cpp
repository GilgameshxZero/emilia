/*
Emilia-tan Script

This script prints the client's IP.
*/

#include "../rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
	_setmode(_fileno(stdout), _O_BINARY);

	std::string response = std::getenv("CLIENT_IP");

	std::cout << "HTTP/1.1 200 OK\r\n"
		<< "content-type:text/html\r\n"
		<< "\r\n"
		<< response;

	return 0;
}