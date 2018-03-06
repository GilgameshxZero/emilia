#include "Main.h"

int onProcessMessage(void *funcParam) {
	Rain::WSA2RecvParam &recvParam = *reinterpret_cast<Rain::WSA2RecvParam *>(funcParam);
	std::cout << recvParam.message;

	return 0;
}

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
	Rain::WSA2RecvParam recvParam;

	recvParam.bufLen = 1024;
	recvParam.funcParam = reinterpret_cast<void *>(&recvParam);
	recvParam.message = new std::string();
	recvParam.onProcessMessage = onProcessMessage;
	recvParam.onRecvInit = NULL;
	recvParam.onRecvEnd = NULL;
	recvParam.socket = &cSocket;

	CreateThread(NULL, 0, Rain::recvThread, reinterpret_cast<void *>(&recvParam), NULL, NULL);

	std::cout << "Client IP:\t" << Rain::getClientNumIP(cSocket) << "\n";

	std::string message = "220 emilia-tan.com Simple Mail Transfer Service Ready\r\n\n\n";
	Rain::sendText(cSocket, message.c_str(), message.length());

	std::cin.get();

	return 0;
}