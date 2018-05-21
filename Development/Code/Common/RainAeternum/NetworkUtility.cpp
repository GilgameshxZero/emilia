#include "NetworkUtility.h"

namespace Rain {
	int initWinsock(WSADATA &wsaData) {
		return WSAStartup(MAKEWORD(2, 2), &wsaData);
	}

	int getClientAddr(std::string host, std::string port, struct addrinfo **result) {
		struct addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		int ret = getaddrinfo(host.c_str(), port.c_str(), &hints, result);
		if (ret != 0)
			WSACleanup();
		return ret;
	}

	int createClientSocket(struct addrinfo **ptr, SOCKET &ConnectSocket) {
		ConnectSocket = INVALID_SOCKET;

		ConnectSocket = socket((*ptr)->ai_family, (*ptr)->ai_socktype,
			(*ptr)->ai_protocol);

		if (ConnectSocket == INVALID_SOCKET) {
			int ret = WSAGetLastError();
			freeaddrinfo(*ptr);
			WSACleanup();
			return ret;
		}
		return 0;
	}

	int connToServ(struct addrinfo **ptr, SOCKET &ConnectSocket) {
		struct addrinfo *curaddr = (*ptr);
		int ret;

		while (true) {
			ret = connect(ConnectSocket, curaddr->ai_addr, (int) curaddr->ai_addrlen);
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

	int shutdownSocketSend(SOCKET &ConnectSocket) {
		if (shutdown(ConnectSocket, SD_SEND) == SOCKET_ERROR) {
			int ret = WSAGetLastError();
			closesocket(ConnectSocket);
			WSACleanup();
			return ret;
		}

		return 0;
	}

	int getServAddr(std::string port, struct addrinfo **result) {
		struct addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		int ret = getaddrinfo(NULL, port.c_str(), &hints, result);
		if (ret != 0)
			WSACleanup();
		return ret;
	}

	int createServLSocket(struct addrinfo **ptr, SOCKET &ListenSocket) {
		ListenSocket = socket((*ptr)->ai_family, (*ptr)->ai_socktype, (*ptr)->ai_protocol);

		if (ListenSocket == INVALID_SOCKET) {
			int ret = WSAGetLastError();
			freeaddrinfo(*ptr);
			WSACleanup();
			return ret;
		}
		return 0;
	}

	int bindServLSocket(struct addrinfo **ptr, SOCKET &ListenSocket) {
		if (bind(ListenSocket, (*ptr)->ai_addr, (int) (*ptr)->ai_addrlen) == SOCKET_ERROR) {
			int ret = WSAGetLastError();
			freeaddrinfo(*ptr);
			closesocket(ListenSocket);
			WSACleanup();
			return ret;
		}

		freeaddrinfo(*ptr);
		return 0;
	}

	int listenServSocket(SOCKET &ListenSocket) {
		if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
			int ret = WSAGetLastError();
			closesocket(ListenSocket);
			WSACleanup();
			return ret;
		}
		return 0;
	}

	int servAcceptClient(SOCKET &ClientSocket, SOCKET &ListenSocket) {
		ClientSocket = INVALID_SOCKET;

		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			int ret = WSAGetLastError();
			closesocket(ListenSocket);
			WSACleanup();
			return ret;
		}
		return 0;
	}

	std::string getClientNumIP(SOCKET &clientsock) {
		static struct sockaddr clname;
		static int clnamesize;
		static TCHAR clhname[32];

		clnamesize = sizeof(clname);
		getpeername(clientsock, &clname, &clnamesize);
		getnameinfo(&clname, clnamesize, clhname, 32, NULL, NULL, NI_NUMERICHOST);

		return std::string(clhname);
	}

	int quickClientInit(WSADATA &wsaData, std::string host, std::string port, struct addrinfo **paddr, SOCKET &connect) {
		int error;
		error = Rain::initWinsock(wsaData);
		if (error) return error;
		error = Rain::getClientAddr(host, port, paddr);
		if (error) return error;
		error = Rain::createClientSocket(paddr, connect);
		if (error) return error;
		return 0;
	}

	int quickServerInit(WSADATA &wsaData, std::string port, struct addrinfo **paddr, SOCKET &listener) {
		int error;
		error = Rain::initWinsock(wsaData);
		if (error) return error;
		error = Rain::getServAddr(port, paddr);
		if (error) return error;
		error = Rain::createServLSocket(paddr, listener);
		if (error) return error;
		error = Rain::bindServLSocket(paddr, listener);
		if (error) return error;
		return 0;
	}

	int sendText(SOCKET &sock, const char *cstrtext, long long len) {
		long long sent = 0;
		int ret;

		while (sent < len) {
			ret = send(sock, cstrtext + sent, static_cast<int>(len - sent), 0);
			if (ret == SOCKET_ERROR) {
				ret = WSAGetLastError();
				return ret;
			}

			sent += ret;
		}

		return 0;
	}
	int sendText(SOCKET &sock, std::string strText) {
		return sendText(sock, strText.c_str(), strText.length());
	}
	int sendTextRef(SOCKET &sock, std::string &strText) {
		return sendText(sock, strText.c_str(), strText.length());
	}

	int sendBlockText(SOCKET &sock, std::string strText) {
		return sendText(sock, Rain::tToStr(strText.length()) + " " + strText);
	}
	int sendBlockTextRef(SOCKET &sock, std::string &strText) {
		return sendText(sock, Rain::tToStr(strText.length()) + " " + strText);
	}

	int sendHeader(SOCKET &sock, std::unordered_map<std::string, std::string> *headers) {
		std::string message;
		for (std::unordered_map<std::string, std::string>::iterator it = headers->begin(); it != headers->end(); it++)
			message += it->first + ": " + it->second + "\n";
		message += "\n";
		return Rain::sendText(sock, message.c_str(), message.length());
	}

	RainWindow *createSendHandler(std::unordered_map<UINT, RainWindow::MSGFC> *msgm) {
		RainWindow *rw = new RainWindow();
		rw->create(msgm, NULL, NULL, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, "", NULL, NULL, "", WS_POPUP, 0, 0, 0, 0, NULL, NULL, RainWindow::NULLCLASSNAME);

		return rw;
	}
}