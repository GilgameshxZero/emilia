/*
Emilia-tan Script

This script prints out the GET query string.
*/

#include "rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
    _setmode(_fileno(stdout), _O_BINARY);

    std::string response;
    std::map<std::string, std::string> query = Rain::getQueryToMap(std::getenv("QUERY_STRING"));

    for (auto it = query.begin(); it != query.end(); it++) {
		response += it->first + ": " + it->second + Rain::CRLF;
	}

	std::cout << "HTTP/1.1 200 OK" << Rain::CRLF
		<< "content-type:text/html" << Rain::CRLF
		<< Rain::CRLF
		<< response;

    return 0;
}
