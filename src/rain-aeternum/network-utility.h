/*
Standard
*/

#pragma once

#include "network-base.h"
#include "network-recv-param.h"
#include "network-socket-manager.h"
#include "rain-window.h"
#include "utility-string.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace Rain {
	//test if WSAStartup has beencalled
	bool WSAStarted();

	//both sides
	std::string getClientNumIP(SOCKET &clientSock);

	//send message.length() + ' ' + message over a SocketManager
	void sendBlockMessage(Rain::SocketManager &manager, std::string message);
	void sendBlockMessage(Rain::SocketManager &manager, std::string *message);

	//converts a header map into a HTTP header string
	std::string mapToHTTPHeader(std::map<std::string, std::string> &hMap);
	std::string mapToHTTPHeader(std::unordered_map<std::string, std::string> &hMap);

	//create a message queue/window which will respond to messages sent to it
	//RainWindow * which is returned must be freed
	RainWindow *createSendHandler(std::unordered_map<UINT, RainWindow::MSGFC> *msgm);

	//converts a GET query string into a map
	std::map<std::string, std::string> getQueryToMap(std::string query);

	//used to send `headed` messages, similar to `block` messages
	void sendHeadedMessage(Rain::SocketManager &manager, std::string message);
	void sendHeadedMessage(Rain::SocketManager &manager, std::string *message);
}