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

	std::string getClientNumIP(SOCKET &clientSock) {
		static struct sockaddr clname;
		static int clnamesize;
		static TCHAR clhname[32];

		clnamesize = sizeof(clname);
		getpeername(clientSock, &clname, &clnamesize);
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

	std::map<std::string, std::string> getQueryToMap(std::string query) {
		std::size_t start, end;
		std::vector<std::string> lines;

		start = 0;
		end = query.find('&');
		while (end != std::string::npos) {
			lines.push_back(query.substr(start, end - start));
			start = end + 1;
			end = query.find('&', start);
		}
		lines.push_back(query.substr(start, end));

		std::map<std::string, std::string> rt;
		for (std::size_t a = 0; a < lines.size(); a++) {
			std::size_t equals = lines[a].find('=');
			rt.insert(std::make_pair(lines[a].substr(0, equals), lines[a].substr(equals + 1, lines[a].length())));
		}

		return rt;
	}

	RecvHandlerParam::EventHandler onHeadedConnect(RecvHandlerParam::EventHandler delegate) {
		return delegate;
	}
	RecvHandlerParam::EventHandler onHeadedMessage(RecvHandlerParam::EventHandler delegate) {
		return [](void *param) {
			return 0;
		};
	}
	RecvHandlerParam::EventHandler onHeadedDisconnect(RecvHandlerParam::EventHandler delegate) {
		return delegate;
	}
	void sendHeadedMessage(Rain::SocketManager &manager, std::string message) {

	}
	void sendHeadedMessage(Rain::SocketManager &manager, std::string *message) {

	}
}