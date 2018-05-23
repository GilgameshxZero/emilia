/*
Standard
*/

#pragma once
#include "NetworkBase.h"
#include "NetworkClientManager.h"
#include "RainWindow.h"

#include <string>
#include <unordered_map>

namespace Rain {
	//server side functions
	int getServAddr(std::string port, struct addrinfo **result);
	int createServLSocket(struct addrinfo **ptr, SOCKET &ListenSocket);
	int bindServLSocket(struct addrinfo **ptr, SOCKET &ListenSocket); //frees ptr
	int listenServSocket(SOCKET &ListenSocket);
	int servAcceptClient(SOCKET &ClientSocket, SOCKET &ListenSocket);

	//both sides
	std::string getClientNumIP(SOCKET &clientsock);

	//custom Rain formats of messages
	int sendBlockText(SOCKET &sock, std::string strText);
	int sendBlockTextRef(SOCKET &sock, std::string &strText);

	void sendBlockMessage(Rain::NetworkSocketManager &manager, std::string message);
	void sendBlockMessage(Rain::NetworkSocketManager &manager, std::string *message);

	int sendHeader(SOCKET &sock, std::unordered_map<std::string, std::string> *headers);

	//create a message queue/window which will respond to messages sent to it
	//RainWindow * which is returned must be freed
	RainWindow *createSendHandler(std::unordered_map<UINT, RainWindow::MSGFC> *msgm);

	//test if WSAStartup called
	bool isWSAStarted();
}