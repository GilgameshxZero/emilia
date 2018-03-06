#define _SCL_SECURE_NO_WARNINGS

#include "Main.h"

struct RecvThreadParam {
	Rain::WSA2RecvParam *recvParam;
};

int onProcessMessage(void *funcParam) {
	RecvThreadParam &rtParam = *reinterpret_cast<RecvThreadParam *>(funcParam);
	std::cout << *rtParam.recvParam->message << std::endl;

	return 0;
}

void onRecvEnd(void *funcParam) {
	std::cout << "server shutdown\n";
}

std::string charToHex(char c) {
	std::stringstream ss;
	ss << std::hex << std::setfill('0') << std::setw(2) << (int) static_cast<unsigned char>(c);
	return ss.str();
}

void appHexByte(std::string &data, std::string hex) {
	data.push_back((char)(hex))
}

int main() {
	WSADATA wsaData;
	struct addrinfo *sAddr;
	SOCKET cSocket;
	int error = Rain::quickClientInit(wsaData, "smtp.gmail.com", "25", &sAddr, cSocket);
	if (error) {
		Rain::reportError(error, "error in quickClientInit");
		return error;
	}
	error = Rain::connToServ(&sAddr, cSocket);
	if (error) {
		Rain::reportError(error, "error in connToServ");
		return error;
	}
	std::cout << "Client initialized\n";

	Rain::WSA2RecvParam recvParam;
	RecvThreadParam rtParam;

	rtParam.recvParam = &recvParam;

	recvParam.bufLen = 65536;
	recvParam.funcParam = reinterpret_cast<void *>(&rtParam);
	recvParam.message = new std::string();
	recvParam.onProcessMessage = onProcessMessage;
	recvParam.onRecvInit = NULL;
	recvParam.onRecvEnd = onRecvEnd;
	recvParam.socket = &cSocket;

	CreateThread(NULL, 0, Rain::recvThread, reinterpret_cast<void *>(&recvParam), NULL, NULL);

	std::string &m = *recvParam.message;

	Sleep(500);
	std::string message = "EHLO smtp.emilia-tan.com\r\n";
	Rain::sendText(cSocket, message.c_str(), message.length());

	Sleep(500);
	message = "STARTTLS\r\n";
	Rain::sendText(cSocket, message.c_str(), message.length());

	Sleep(500);
	char *cArr;
	cArr = new char[32];
	time_t secNow = std::time(NULL);
	int iSecNow = secNow;
	std::cout << "time: " << iSecNow << std::endl;
	std::copy(static_cast<const char*>(static_cast<const void*>(&iSecNow)),
			  static_cast<const char*>(static_cast<const void*>(&iSecNow)) + sizeof iSecNow, cArr);
	for (int a = 0; a < 2; a++)
		std::swap(cArr[a], cArr[3 - a]);
	for (int a = 4; a < 32; a++)
		cArr[a] = static_cast<char>(static_cast<unsigned char>(rand() % 256));
	std::cout << "send:\n";
	for (int a = 0; a < 32; a++)
		std::cout << charToHex(static_cast<char>(cArr[a])) << "\t";
	std::cout << std::endl;
	Rain::sendText(cSocket, cArr, 32);
	delete[] cArr;

	Sleep(500);
	std::cout << "receive:\n";
	for (int a = 0; a < m.length(); a++)
		std::cout << charToHex(m[a]) << "\t";
	std::cout << std::endl;

	std::cin.get();

	return 0;
}