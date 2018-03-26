/*
Standard
*/

/*
Implements thread for listening for socket connections and spawning recvThreads.
*/

#pragma once

#include "RainWSA2SendRecv.h"

#include <cstddef>

namespace Rain {
	//creates thread which accepts on a specified listening socket, until the socket is closed from outside the thread
	//accepted connections will spawn a recvThread for each connection with specified parameters
	//when listening socket is closed, thread will close all connection sockets and wait until all spawned recvThreads exit
	HANDLE createListenThread(SOCKET lSocket,
							  std::size_t recvBufferLength,
							  void *recvFuncParam, //passed to the following three functions, packaged with the socket and the message
							  Rain::WSA2RecvPMFunc onProcessMessage,
							  Rain::WSA2RecvInitFunc onRecvInit,
							  Rain::WSA2RecvExitFunc onRecvExit);
	
	DWORD WINAPI listenThread(LPVOID lpParameter);
}