/*
Deprecated
*/

/*This library is standardized and regulated, for providing shorcuts for winsock-related actions.
Includes full error-checking and error-management functions.
Resource management is mostly dealt-with within the functions - though WSA Cleanup may need to be called at exit.*/

#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#include <string>

#pragma comment(lib, "Ws2_32.lib")

namespace Rain
{
	int InitWinsock (WSADATA &wsaData);

	//client side functions
	int GetClientAddr (std::string host, std::string port, struct addrinfo **result);
	int CreateClientSocket (struct addrinfo **ptr, SOCKET &ConnectSocket);
	int ConnToServ (struct addrinfo **ptr, SOCKET &ConnectSocket); //does not free ptr or WSACleanup if error
	int ShutdownSocketSend (SOCKET &ConnectSocket);

	//server side functions
	int GetServAddr (std::string port, struct addrinfo **result);
	int CreateServLSocket (struct addrinfo **ptr, SOCKET &ListenSocket);
	int BindServLSocket (struct addrinfo **ptr, SOCKET &ListenSocket); //frees ptr
	int ListenServSocket (SOCKET &ListenSocket);
	int ServAcceptClient (SOCKET &ClientSocket, SOCKET &ListenSocket);

	//uility
	std::string GetClientNumIP (SOCKET &clientsock);
}