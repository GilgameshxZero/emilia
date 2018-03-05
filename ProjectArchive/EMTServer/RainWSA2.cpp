#include "RainWSA2.h"

namespace Rain
{
	int InitWinsock (WSADATA &wsaData)
	{
		return WSAStartup (MAKEWORD (2, 2), &wsaData);
	}

	int GetClientAddr (std::string host, std::string port, struct addrinfo **result)
	{
		struct addrinfo hints;

		ZeroMemory (&hints, sizeof (hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		int ret = getaddrinfo (host.c_str (), port.c_str (), &hints, result);
		if (ret != 0)
			WSACleanup ();
		return ret;
	}

	int CreateClientSocket (struct addrinfo **ptr, SOCKET &ConnectSocket)
	{
		ConnectSocket = INVALID_SOCKET;

		// Create a SOCKET for connecting to server
		ConnectSocket = socket ((*ptr)->ai_family, (*ptr)->ai_socktype,
			(*ptr)->ai_protocol);

		if (ConnectSocket == INVALID_SOCKET)
		{
			int ret = WSAGetLastError ();
			freeaddrinfo (*ptr);
			WSACleanup ();
			return ret;
		}
		return 0;
	}

	int ConnToServ (struct addrinfo **ptr, SOCKET &ConnectSocket)
	{
		struct addrinfo *curaddr = (*ptr);
		int ret;

		// Connect to server. try using next addrinfo structures if one doesn't work
		while (true)
		{
			ret = connect (ConnectSocket, curaddr->ai_addr, (int)curaddr->ai_addrlen);
			if (ret == SOCKET_ERROR)
				ret = WSAGetLastError ();
			else
				break;

			if (curaddr->ai_next == NULL)
				break;

			curaddr = curaddr->ai_next;
		}

		//freeaddrinfo (*ptr);

		if (ret)
		{
			//WSACleanup ();
			//return 1;
			return ret;
		}
		return 0;
	}

	int ShutdownSocketSend (SOCKET &ConnectSocket)
	{
		// shutdown the send half of the connection since no more data will be sent
		if (shutdown (ConnectSocket, SD_SEND) == SOCKET_ERROR)
		{
			int ret = WSAGetLastError ();
			closesocket (ConnectSocket);
			WSACleanup ();
			return ret;
		}

		return 0;
	}

	int GetServAddr (std::string port, struct addrinfo **result)
	{
		struct addrinfo hints;

		ZeroMemory (&hints, sizeof (hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the local address and port to be used by the server
		int ret = getaddrinfo (NULL, port.c_str (), &hints, result);
		if (ret != 0)
			WSACleanup ();
		return ret;
	}

	int CreateServLSocket (struct addrinfo **ptr, SOCKET &ListenSocket)
	{
		// Create a SOCKET for the server to listen for client connections
		ListenSocket = socket ((*ptr)->ai_family, (*ptr)->ai_socktype, (*ptr)->ai_protocol);

		if (ListenSocket == INVALID_SOCKET)
		{
			int ret = WSAGetLastError ();
			freeaddrinfo (*ptr);
			WSACleanup ();
			return ret;
		}
		return 0;
	}

	int BindServLSocket (struct addrinfo **ptr, SOCKET &ListenSocket)
	{
		// Setup the TCP listening socket
		if (bind (ListenSocket, (*ptr)->ai_addr, (int)(*ptr)->ai_addrlen) == SOCKET_ERROR)
		{
			int ret = WSAGetLastError ();
			freeaddrinfo (*ptr);
			closesocket (ListenSocket);
			WSACleanup ();
			return ret;
		}

		freeaddrinfo (*ptr);
		return 0;
	}

	int ListenServSocket (SOCKET &ListenSocket)
	{
		if (listen (ListenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			int ret = WSAGetLastError ();
			closesocket (ListenSocket);
			WSACleanup ();
			return ret;
		}
		return 0;
	}

	int ServAcceptClient (SOCKET &ClientSocket, SOCKET &ListenSocket)
	{
		ClientSocket = INVALID_SOCKET;

		// Accept a client socket
		ClientSocket = accept (ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			int ret = WSAGetLastError ();
			closesocket (ListenSocket);
			WSACleanup ();
			return ret;
		}
		return 0;
	}

	std::string GetClientNumIP (SOCKET &clientsock)
	{
		static struct sockaddr clname;
		static int clnamesize;
		static TCHAR clhname[32];

		clnamesize = sizeof (clname);
		getpeername (clientsock, &clname, &clnamesize);
		getnameinfo (&clname, clnamesize, clhname, 32, NULL, NULL, NI_NUMERICHOST);

		return std::string (clhname);
	}
}