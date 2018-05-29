/*
Standard
*/

/*
Implements thread for listening for socket connections and spawning recvThreads.
*/

#pragma once

#include "RainError.h"
#include "RainWSA2SendRecv.h"
#include "RainWSA2Utility.h"

#include <cstddef>
#include <mutex>

namespace Rain {
	//used to pass parameters from createListenThread to listenThread
	struct WSA2ListenThreadParam {
		//same variables as createListenThread - check there for what these do
		SOCKET *lSocket;
		void *recvFuncParam;
		std::size_t recvBufferLength;
		WSA2RecvPMFunc onProcessMessage;
		WSA2RecvInitFunc onRecvInit;
		WSA2RecvExitFunc onRecvExit;
	};

	//used to pass parameters from listenThread spawned recvThreads to recv handlers specified in createListenThread
	struct WSA2ListenThreadRecvFuncDelegateParam {
		//socket used by this recvThread
		SOCKET *cSocket;

		//message used by the recvThread
		std::string message;

		//parameter to be dynamically set and used by the delegate recv handlers
		void *delegateParam;

		//parameter passed from createListenThread
		void *callerParam;
	};

	//used to pass parameters from listenThread to spawned recvThreads
	struct WSA2ListenThreadRecvFuncParam {
		//used by recvThread handlers to pass to their delegate handlers
		WSA2ListenThreadRecvFuncDelegateParam ltrfdParam;

		//keep track of WSA2RecvFuncParam, which needs to be freed on recvThread exit
		WSA2RecvFuncParam *pRFParam;

		//position in a linked list of ltrfParam, which should be modified when recvThread inits or exits
		WSA2ListenThreadRecvFuncParam *prevLTRFP, *nextLTRFP;

		//mutex to lock when changing linked list
		//todo: don't lock whole linked list - just the positions around where we are modifying
		std::mutex *llMutex;

		//access WSA2ListenThreadParam to access the delegate handlers
		WSA2ListenThreadParam *pLTParam;

		//handle to the current recvThread for use when waiting on end
		HANDLE hRecvThread;
	};

	//creates thread which accepts on a specified listening socket, until the socket is closed from outside the thread
	//accepted connections will spawn a recvThread for each connection with specified parameters
	//any parameters that the onProc functions will need to use will need to be created in onRecvInit - there will be space to store the parameters to be passed on the functions through the use of a void * in ListenThreadRecvParam
	//when listening socket is closed, thread will close all connection sockets and wait until all spawned recvThreads exit
	HANDLE createListenThread(SOCKET &lSocket,
							  void *recvFuncParam, //parameter to be passed to handler delegates
							  std::size_t recvBufferLength,
							  Rain::WSA2RecvPMFunc onProcessMessage,
							  Rain::WSA2RecvInitFunc onRecvInit,
							  Rain::WSA2RecvExitFunc onRecvExit);
	
	//don't use externally
	//implements the listen thread specified above
	DWORD WINAPI listenThread(LPVOID lpParameter);

	//handlers used by the listenThread, which eventually call the corresponding handlers passed to createListenThread
	void onListenThreadRecvInit(void *funcParam);
	void onListenThreadRecvExit(void *funcParam);
	int onListenThreadRecvProcessMessage(void *funcParam);
}