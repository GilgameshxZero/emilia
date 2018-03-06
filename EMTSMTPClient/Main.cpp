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

int hexDigToInt(char hex) {
	if (hex >= '0' && hex <= '9')
		return hex - '0';
	return hex - 'a' + 10;
}

char hexToChar(std::string hex) {
	unsigned char byte = static_cast<unsigned char>(hexDigToInt(hex[0]) * 16 + hexDigToInt(hex[1]));
	return reinterpret_cast<char &>(byte);
}

std::string charToHex(char c) {
	std::stringstream ss;
	ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(reinterpret_cast<unsigned char &>(c));
	return ss.str();
}

std::string &appHexByte(std::string &data, std::string hex) {
	data.push_back(hexToChar(hex));
	return data;
}

std::string &appHexBytes(std::string &data, std::string hexStr) {
	for (int a = 0; a < hexStr.length(); a += 2)
		appHexByte(data, hexStr.substr(a, 2));
	return data;
}

template <typename T>
std::string tToByteStr(T x) {
	std::string ret;
	int xLen = sizeof x;
	char *xC = reinterpret_cast<char *>(static_cast<void *>(&x));
	for (int a = 0; a < xLen; a++)
		ret.push_back(xC[a]);
	return ret;
}

std::string byteStrToHexStr(std::string data) {
	std::stringstream ss;
	for (int a = 0; a < data.length(); a++)
		ss << charToHex(data[a]);
	return ss.str();
}

std::string reverseByteStr(std::string byteStr) {
	std::reverse(byteStr.begin(), byteStr.end());
	return byteStr;
}

int main() {
	std::string server = "hotmail-com.olc.protection.outlook.com";//"outgoing.mit.edu";// "smtp.gmail.com";

	WSADATA wsaData;
	struct addrinfo *sAddr;
	SOCKET cSocket;
	int error = Rain::quickClientInit(wsaData, server, "25", &sAddr, cSocket);
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

	Sleep(200);
	std::string message = "EHLO smtp.emilia-tan.com\r\n";
	m.clear();
	Rain::sendText(cSocket, message.c_str(), message.length());

	Sleep(200);
	message = "MAIL FROM:<gil@emilia-tan.com>\r\n";
	m.clear();
	Rain::sendText(cSocket, message.c_str(), message.length());

	Sleep(200);
	message = "RCPT TO:<wfyan2002@hotmail.com>\r\n";
	m.clear();
	Rain::sendText(cSocket, message.c_str(), message.length());

	Sleep(200);
	message = "DATA\r\n";
	m.clear();
	Rain::sendText(cSocket, message.c_str(), message.length());

	Sleep(200);
	message = Rain::readFullFile("data.txt", message);
	m.clear();
	Rain::sendText(cSocket, message.c_str(), message.length());

	Sleep(200);
	message = "QUIT\r\n";
	m.clear();
	Rain::sendText(cSocket, message.c_str(), message.length());
	/*
	Sleep(200);
	message = "STARTTLS\r\n";
	m.clear();
	Rain::sendText(cSocket, message.c_str(), message.length());

	Sleep(200);
	message.clear();
	std::stringstream ss;
	ss << charToHex(22) //ContentType handshake
		<< charToHex(3) << charToHex(3) //ProtocolVersion 3.3
		<< 
	appHexBytes(message, ss.str());
	appHexBytes(message, "0303");
	appHexBytes(message, byteStrToHexStr(reverseByteStr(tToByteStr(static_cast<unsigned int>(std::time(NULL)))))); //big endian
	appHexBytes(message, "00000000000000000000000000000000000000000000000000000000"); //28 bytes
	appHexBytes(message, "0001");
	appHexBytes(message, "01");
	std::cout << "send:\n" << byteStrToHexStr(message) << std::endl;
	m.clear();
	Rain::sendText(cSocket, message.c_str(), message.length());

	Sleep(200);
	std::cout << "receive:\n" << byteStrToHexStr(m) << std::endl;
	*/
	std::cin.get();

	return 0;
}