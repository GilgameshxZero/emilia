/*
Emilia-tan Script

This script modifies data\prototype-radio-current.ini to contain information about what is currently playing in the radio and keeps it up-to-date.
*/

#include "../rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);

	srand(time(NULL));

	//only one instance can run at a single time


	std::string response;
	std::cout << "HTTP/1.1 200 OK" << Rain::CRLF
		<< "content-type:audio/mpeg" << Rain::CRLF
		<< Rain::CRLF;
	std::cout.flush();

	return 0;
}