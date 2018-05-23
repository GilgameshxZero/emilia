/*
Standard
*/

#pragma once
#include "NetworkWSAInclude.h"
#include "RainWindow.h"

#include <string>
#include <unordered_map>

namespace Rain {
	//shortcut function for starting WSA2.2
	int initWinsock22();

	//target returned needs to be freed
	//if no error, returns 0
	int getTargetAddr(struct addrinfo **target,  std::string host, std::string port,
					  int family = AF_UNSPEC, int sockType = SOCK_STREAM, int type = IPPROTO_TCP);

	//if no error, returns 0
	int createSocket(SOCKET &newSocket,
					 int family = AF_UNSPEC, int sockType = SOCK_STREAM, int type = IPPROTO_TCP);

	//run through all the target possibilities and try to connect to any of them
	//if no error, returns 0
	int connectTarget(SOCKET &cSocket, struct addrinfo **target);

	//if no error, returns 0
	int shutdownSocketSend(SOCKET &cSocket);

	//send raw message over a socket
	int sendRawMessage(SOCKET &sock, const char *cstrtext, int len);
	int sendRawMessage(SOCKET &sock, std::string strText);
	int sendRawMessage(SOCKET &sock, std::string *strText);
}