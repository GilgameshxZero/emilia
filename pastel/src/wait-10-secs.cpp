/*
Emilia-tan Script

This script waits for 10 seconds, then exits.
*/

#include "rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);

	std::string response = "Waiting for 10 seconds...";

	std::cout << "HTTP/1.1 200 OK" << Rain::CRLF
		<< "content-type:text/html" << Rain::CRLF
		<< Rain::CRLF
		<< response;
	std::cout.flush();
	//_close(_fileno(stdout));

	std::this_thread::sleep_for(std::chrono::milliseconds(10000));

	return 0;
}
