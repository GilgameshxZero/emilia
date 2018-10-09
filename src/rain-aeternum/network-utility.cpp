#include "network-utility.h"

namespace Rain {
	bool WSAStarted() {
		SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED) {
			return false;
		}
		closesocket(s);
		return true;
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
	
	void sendBlockMessage(Rain::SocketManager &manager, std::string message) {
		Rain::sendBlockMessage(manager, &message);
	}
	void sendBlockMessage(Rain::SocketManager &manager, std::string *message) {
		manager.sendRawMessage(Rain::tToStr(message->length()) + " ");
		manager.sendRawMessage(message);
	}

	std::string mapToHTTPHeader(std::map<std::string, std::string> &hMap) {
		std::string message;
		for (auto it = hMap.begin(); it != hMap.end(); it++)
			message += it->first + ": " + it->second + "\n";
		return message + "\n";
	}
	std::string mapToHTTPHeader(std::unordered_map<std::string, std::string> &hMap) {
		std::string message;
		for (auto it = hMap.begin(); it != hMap.end(); it++)
			message += it->first + ": " + it->second + "\n";
		return message + "\n";
	}

	RainWindow *createSendHandler(std::unordered_map<UINT, RainWindow::MSGFC> *msgm) {
		RainWindow *rw = new RainWindow();
		rw->create(msgm, NULL, NULL, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, "", NULL, NULL, "", WS_POPUP, 0, 0, 0, 0, NULL, NULL, RainWindow::NULLCLASSNAME);

		return rw;
	}
}