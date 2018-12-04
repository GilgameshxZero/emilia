/*
Standard
*/

#pragma once

#include "network-base.h"
#include "network-recv-handler-param.h"
#include "network-socket-manager.h"
#include "rain-window.h"

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

	//three predefined handlers for NetworkSocketManagers for convenient socket use for derived applications
	//these handlers take delegate handlers of their own; the main difference is that onMessage only calls its delegate when it receives a headed message
	//the concept of a headed message is similar to that of a `block` message above; it contains the length of the message as a header
	//the length header is two bytes big endian (most sign. dig. first), or, if both are zero, then a 4-byte integer big endian which follows (6 bytes total header); the message follows immediately after the header
	//use with bind in functional
	RecvHandlerParam::EventHandler onHeadedConnect(RecvHandlerParam::EventHandler delegate);
	RecvHandlerParam::EventHandler onHeadedMessage(RecvHandlerParam::EventHandler delegate);
	RecvHandlerParam::EventHandler onHeadedDisconnect(RecvHandlerParam::EventHandler delegate);
	void sendHeadedMessage(Rain::SocketManager &manager, std::string message);
	void sendHeadedMessage(Rain::SocketManager &manager, std::string *message);
}