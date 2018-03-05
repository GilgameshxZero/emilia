#pragma once

#include "../RainLibrary3/RainLibraries.h"

//parameter passed to any listening thread
//one for each ListenThread
struct ListenThreadParam {
	//persistent socket on which to listen for clients
	SOCKET *lSocket;

	//mutex to lock the link list when modifications are being made to it
	std::mutex *ltLLMutex;

	//window representing this thread's message queue, implemented by RainWindow
	Rain::RainWindow rainWnd;

	//current thread handle
	HANDLE hThread;

	//pointer to ListenThreadParams before and after this, in a linked list
	ListenThreadParam *prevLTP, *nextLTP;

	//parameters to pass to message procs
	std::string *serverRootDir;
	std::string *serverAux;

	SOCKET clientsock;
	Rain::WSA2RecvParam recvparam;
	HANDLE hrecvthread;
};