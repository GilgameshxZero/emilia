/*
Emilia-tan Script

This script prints out the POST query string.
*/

#include "../rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
    _setmode(_fileno(stdout), _O_BINARY);

    static const int BUFFER_SIZE = 65536;
    char *buffer = new char[BUFFER_SIZE];
    std::string response;
    while (std::cin) {
        std::cin.read(buffer, BUFFER_SIZE);
		response += std::string(buffer, static_cast<std::size_t>(std::cin.gcount()));
    };
	delete[] buffer;

    std::cout << "HTTP/1.1 200 OK\r\n"
              << "content-type:text/html\r\n"
              << "\r\n"
              << response;

    return 0;
}