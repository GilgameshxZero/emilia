/*
Deprecated
*/

#pragma once

#include "RainWSA2.h"
#include "Utility.h"
#include <iostream>
#include <queue>
#include <string>
#include <sstream>
#include <Windows.h>

namespace Rain
{
	struct RecvParam
	{
		SOCKET *sock;
		std::string *message;
		int buflen;

		//parameter for external functions (the following two functions)
		void *funcparam;

		//called every time a message is received; return nonzero if we should end thread
		typedef int (*PMFunc) (void *);
		PMFunc ProcessMessage;

		//this function is called when we quit RecvThread, possibly due to the other end terminating the connection
		typedef void (*ExitFunc) (void *);
		ExitFunc OnRecvEnd;
	};

	//calls a function whenever a message is received
	DWORD WINAPI RecvThread (LPVOID lpParameter);

	//SendThread is application-defined.

	int SendText (SOCKET &sock, const char *cstrtext, long long len);
}