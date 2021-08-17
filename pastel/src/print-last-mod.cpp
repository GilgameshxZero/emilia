/*
Emilia-tan Script

This script prints the last modified time of project index.
*/

#include "rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
	_setmode(_fileno(stdout), _O_BINARY);

	std::string response = Rain::getTime("%D %T%z", Rain::getFileLastModifyTime("../../.emilia/index.idx"));

	std::cout << "HTTP/1.1 200 OK" << Rain::CRLF
		<< "content-type:text/html" << Rain::CRLF
		<< Rain::CRLF
		<< response;

	return 0;
}
