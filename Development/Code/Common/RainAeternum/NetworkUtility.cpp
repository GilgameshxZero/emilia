#include "NetworkUtility.h"

namespace Rain {

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


	int sendBlockText(SOCKET &sock, std::string strText) {
		return sendRawMessage(sock, Rain::tToStr(strText.length()) + " " + strText);
	}
	int sendBlockTextRef(SOCKET &sock, std::string &strText) {
		return sendRawMessage(sock, Rain::tToStr(strText.length()) + " " + strText);
	}

	void sendBlockMessage(Rain::NetworkSocketManager &manager, std::string message) {
		Rain::sendBlockMessage(manager, &message);
	}
	void sendBlockMessage(Rain::NetworkSocketManager &manager, std::string *message) {
		manager.sendRawMessage(Rain::tToStr(message->length()) + " ");
		manager.sendRawMessage(message);
	}

	int sendHeader(SOCKET &sock, std::unordered_map<std::string, std::string> *headers) {
		std::string message;
		for (std::unordered_map<std::string, std::string>::iterator it = headers->begin(); it != headers->end(); it++)
			message += it->first + ": " + it->second + "\n";
		message += "\n";
		return Rain::sendRawMessage(sock, message.c_str(), static_cast<int>(message.length()));
	}

	RainWindow *createSendHandler(std::unordered_map<UINT, RainWindow::MSGFC> *msgm) {
		RainWindow *rw = new RainWindow();
		rw->create(msgm, NULL, NULL, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, "", NULL, NULL, "", WS_POPUP, 0, 0, 0, 0, NULL, NULL, RainWindow::NULLCLASSNAME);

		return rw;
	}

	bool isWSAStarted() {
		SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED) {
			return false;
		}
		closesocket(s);
		return true;
	}
}