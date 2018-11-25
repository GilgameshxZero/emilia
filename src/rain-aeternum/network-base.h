/*
Standard
*/

/*
Basic functions for connecting sockets and sending over them.

Higher-level, optional functions are in NetworkUtility.
*/

#pragma once
#include "network-wsa-include.h"
#include "rain-window.h"

#include <string>
#include <unordered_map>

namespace Rain {
	//shortcut function for starting WSA2.2
	int initWinsock22();

	//target returned needs to be freed
	//if no error, returns 0
	int getTargetAddr(struct addrinfo **target, std::string host, std::string port,
					  int family = AF_UNSPEC, int sockType = SOCK_STREAM, int type = IPPROTO_TCP);

	//same thing as getTargetAddr, but with different default arguments, to be used for servers trying to listen on a port
	//return 0 if no error
	int getServerAddr(struct addrinfo **server, std::string port,
					  int family = AF_INET, int sockType = SOCK_STREAM, int type = IPPROTO_TCP, int flags = AI_PASSIVE);

	//if no error, returns 0
	int createSocket(SOCKET &newSocket,
					 int family = AF_UNSPEC, int sockType = SOCK_STREAM, int type = IPPROTO_TCP);

	//run through all the target possibilities and try to connect to any of them
	//if no error, returns 0
	int connectTarget(SOCKET &cSocket, struct addrinfo **target);

	//binds a socket to a local address
	//returns 0 if no error
	int bindListenSocket(struct addrinfo **addr, SOCKET &lSocket);

	//returns a new socket for a connected client
	//blocks until a client connects, or until blocking functions are called off
	SOCKET acceptClientSocket(SOCKET &lSocket);

	//frees address gotten by getTargetAddr or getServerAddr
	void freeAddrInfo(struct addrinfo **addr);

	//if no error, returns 0
	int shutdownSocketSend(SOCKET &cSocket);

	//send raw message over a socket
	//returns 0 if no error
	int sendRawMessage(SOCKET &sock, const char *cstrtext, int len);
	int sendRawMessage(SOCKET &sock, std::string strText);
	int sendRawMessage(SOCKET &sock, std::string *strText);
}