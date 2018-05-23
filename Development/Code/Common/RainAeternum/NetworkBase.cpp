#include "NetworkBase.h"

namespace Rain {
	int initWinsock22() {
		static WSADATA wsaData;
		return WSAStartup(MAKEWORD(2, 2), &wsaData);
	}
	int getTargetAddr(struct addrinfo **target, std::string host, std::string port,
					  int family, int sockType, int type) {
		static struct addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = family;
		hints.ai_socktype = sockType;
		hints.ai_protocol = type;

		return getaddrinfo(host.c_str(), port.c_str(), &hints, target);
	}
	int createSocket(SOCKET &newSocket,
					 int family, int sockType, int type) {
		newSocket = socket(family, sockType, type);
		if (newSocket == INVALID_SOCKET)
			return WSAGetLastError();
		return 0;
	}
	int connectTarget(SOCKET &cSocket, struct addrinfo **target) {
		static struct addrinfo *curaddr;
		static int ret;

		curaddr = (*target);
		while (true) {
			ret = connect(cSocket, curaddr->ai_addr, (int) curaddr->ai_addrlen);
			if (ret == SOCKET_ERROR)
				ret = WSAGetLastError();
			else
				break;

			if (curaddr->ai_next == NULL)
				break;
			curaddr = curaddr->ai_next;
		}

		return ret;
	}
	int shutdownSocketSend(SOCKET &cSocket) {
		if (shutdown(cSocket, SD_SEND) == SOCKET_ERROR)
			return WSAGetLastError();
		return 0;
	}

	int sendRawMessage(SOCKET &sock, const char *cstrtext, int len) {
		static int sent, ret;

		sent = ret = 0;
		while (sent < len) {
			ret = send(sock, cstrtext + sent, len - sent, 0);
			if (ret == SOCKET_ERROR) {
				ret = WSAGetLastError();
				return ret;
			}
			sent += ret;
		}
		return ret;
	}
	int sendRawMessage(SOCKET &sock, std::string strText) {
		return sendRawMessage(sock, strText.c_str(), static_cast<int>(strText.length()));
	}
	int sendRawMessage(SOCKET &sock, std::string *strText) {
		return sendRawMessage(sock, strText->c_str(), static_cast<int>(strText->length()));
	}
}