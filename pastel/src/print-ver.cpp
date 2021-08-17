/*
Emilia-tan Script

This script prints the server's version.
*/

#include "rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
	_setmode(_fileno(stdout), _O_BINARY);

	std::string response = std::getenv("EMILIA_VERSION");

	std::cout << "HTTP/1.1 200 OK" << Rain::CRLF
		<< "content-type:text/html" << Rain::CRLF
		<< Rain::CRLF
		<< response;

	return 0;
}
