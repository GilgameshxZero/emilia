/*
Emilia-tan Script

This script prints the environment variables.
*/

#include "rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
	_setmode(_fileno(stdout), _O_BINARY);

	std::string response;

	LPCH curEnvBlock = GetEnvironmentStrings();
	int prevVarBeg = -1;
	for (int a = 0;; a++) {
		if (curEnvBlock[a] == '\0') {
			std::string s = std::string(curEnvBlock + prevVarBeg + 1, curEnvBlock + a);
			response += s + "<br>";
			prevVarBeg = a;
			if (curEnvBlock[a + 1] == '\0')
				break;
		}
	}
	FreeEnvironmentStrings(curEnvBlock);

	std::cout << "HTTP/1.1 200 OK" << Rain::CRLF
		<< "content-type:text/html" << Rain::CRLF
		<< Rain::CRLF
		<< response;

	return 0;
}
