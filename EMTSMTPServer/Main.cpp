#include "Main.h"

int main() {
	WSADATA wsaData;
	struct addrinfo *sAddr;
	SOCKET lSocket;
	int error = Rain::quickServerInit(wsaData, "25", &sAddr, lSocket);
	if (error) {
		Rain::reportError(error, "error in quickServerInit");
		return error;
	}
	std::cout << "Server initialized\n";
	error = Rain::listenServSocket(lSocket);
	if (error) {
		Rain::reportError(error, "error in listenServSocket");
		return error;
	}
	std::cout << "Listening setup\n";

	SOCKET cSocket;
	Rain::servAcceptClient(cSocket, lSocket);

	std::cout << "Client IP:\t" << Rain::getClientNumIP(cSocket) << "\n";

	std::string message = "220 foo.com Simple Mail Transfer Service Ready";
	Rain::sendText(cSocket, message.c_str(), message.length());

	return 0;
}