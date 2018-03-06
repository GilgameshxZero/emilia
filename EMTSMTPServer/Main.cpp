#include "Main.h"

struct RecvThreadParam {
	Rain::WSA2RecvParam *recvParam;
	SOCKET *lSocket;
	int responseNum;
};

int onProcessMessage(void *funcParam) {
	RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
	std::cout << *rtParam.recvParam->message << std::endl;

	std::vector<std::string> responses;
	responses.push_back("250 emilia-tan.com is still best girl!\r\n");
	responses.push_back("250 OK\r\n");
	responses.push_back("250 OK\r\n");
	responses.push_back("354 Start mail input; end with <CRLF>.<CRLF>\r\n");
	responses.push_back("250 OK\r\n");
	responses.push_back("221 emilia-tan.com Service closing transmission channel\r\n");

	Rain::sendText(*rtParam.recvParam->socket, responses[rtParam.responseNum].c_str(), responses[rtParam.responseNum].length());
	rtParam.responseNum++;

	if (rtParam.responseNum == responses.size())
		return 1;

	return 0;
}

void onRecvEnd(void *funcParam) {
	RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);

	SOCKET *cSocket = new SOCKET();
	Rain::servAcceptClient(*cSocket, *rtParam.lSocket);

	Rain::WSA2RecvParam *recvParam = new Rain::WSA2RecvParam();
	RecvThreadParam *newRTParam = new RecvThreadParam();

	newRTParam->lSocket = rtParam.lSocket;
	newRTParam->recvParam = recvParam;
	newRTParam->responseNum = 0;

	recvParam->bufLen = 65536;
	recvParam->funcParam = reinterpret_cast<void *>(newRTParam);
	recvParam->message = new std::string();
	recvParam->onProcessMessage = onProcessMessage;
	recvParam->onRecvInit = NULL;
	recvParam->onRecvEnd = onRecvEnd;
	recvParam->socket = cSocket;

	CreateThread(NULL, 0, Rain::recvThread, reinterpret_cast<void *>(recvParam), NULL, NULL);

	std::cout << "Client IP:\t" << Rain::getClientNumIP(*cSocket) << "\n";

	std::string message = "220 Emilia loves you too!\r\n";
	Rain::sendText(*cSocket, message.c_str(), message.length());
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
	RecvThreadParam rtParam;

	rtParam.lSocket = &lSocket;
	rtParam.recvParam = &recvParam;
	rtParam.responseNum = 0;

	recvParam.bufLen = 65536;
	recvParam.funcParam = reinterpret_cast<void *>(&rtParam);
	recvParam.message = new std::string();
	recvParam.onProcessMessage = onProcessMessage;
	recvParam.onRecvInit = NULL;
	recvParam.onRecvEnd = onRecvEnd;
	recvParam.socket = &cSocket;

	CreateThread(NULL, 0, Rain::recvThread, reinterpret_cast<void *>(&recvParam), NULL, NULL);

	std::cout << "Client IP:\t" << Rain::getClientNumIP(cSocket) << "\n";

	std::string message = "220 Emilia loves you too!\r\n";
	Rain::sendText(cSocket, message.c_str(), message.length());

	std::cin.get();

	return 0;
}