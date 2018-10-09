#include "network-base.h"

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
	int getServerAddr(struct addrinfo **server, std::string port,
					  int family, int sockType, int type, int flags) {
		struct addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = family;
		hints.ai_socktype = sockType;
		hints.ai_protocol = type;
		hints.ai_flags = flags;

		return getaddrinfo(NULL, port.c_str(), &hints, server);
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
	int bindListenSocket(struct addrinfo **addr, SOCKET &lSocket) {
		if (bind(lSocket, (*addr)->ai_addr, (int) (*addr)->ai_addrlen))
			return -1;
		if (listen(lSocket, SOMAXCONN))
			return -1;
		return 0;
	}
	SOCKET acceptClientSocket(SOCKET &lSocket) {
		return accept(lSocket, NULL, NULL);
	}
	void freeAddrInfo(struct addrinfo **addr) {
		freeaddrinfo(*addr);
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