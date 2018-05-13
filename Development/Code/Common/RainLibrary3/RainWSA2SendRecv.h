/*
Standard
*/

#pragma once

#include "RainWindow.h"
#include "RainWSA2Include.h"

#include <string>
#include <unordered_map>

namespace Rain {
	typedef int(*WSA2RecvPMFunc) (void *);
	typedef void(*WSA2RecvInitFunc) (void *);
	typedef void(*WSA2RecvExitFunc) (void *);

	class WSA2RecvFuncParam {
		public:
		WSA2RecvFuncParam();
		WSA2RecvFuncParam(SOCKET *socket, std::string *message, int bufLen, void *funcparam, WSA2RecvPMFunc onProcessMessage, WSA2RecvInitFunc onRecvInit, WSA2RecvExitFunc onRecvExit);

		//socket between server and client
		SOCKET *socket;

		//place where message is stored for use by any RecvFuncs
		std::string *message;

		//length of buffer for recv
		std::size_t bufLen;

		//parameter to be passed to RecvFuncs
		void *funcParam;

		//called whenever recv returns something to the buffer
		//return nonzero to terminate recv loop
		WSA2RecvPMFunc onProcessMessage;

		//called when RecvThread is about to start or end
		WSA2RecvInitFunc onRecvInit;
		WSA2RecvExitFunc onRecvExit;
	};

	//send raw text over a socket
	int sendText(SOCKET &sock, const char *cstrtext, long long len);
	int sendText(SOCKET &sock, std::string strText);

	int sendHeader(SOCKET &sock, std::unordered_map<std::string, std::string> *headers);

	//calls a function whenever a message is received; 
	DWORD WINAPI recvThread(LPVOID lpParameter); //don't use this; use createRecvThread
	HANDLE createRecvThread(
		WSA2RecvFuncParam *recvparam, //if NULL: returns pointer to RecvParam that must be freed when the thread ends
								 //if not NULL: ignores the the next 6 parameters, and uses this as the param for recvThread
		SOCKET *connection = NULL,
		std::string *message = NULL, //where the message is stored each time OnProcessMessage is called
		int buflen = NULL, //the buffer size of the receive function
		void *funcparam = NULL, //additional parameter to pass to the functions OnProcessMessage and OnRecvEnd
		WSA2RecvPMFunc OnProcessMessage = NULL,
		WSA2RecvInitFunc OnRecvInit = NULL, //called at the beginning of thread
		WSA2RecvExitFunc OnRecvEnd = NULL, //called when the other side shuts down send
		DWORD dwCreationFlags = 0,
		SIZE_T dwStackSize = 0,
		LPDWORD lpThreadId = NULL,
		LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL);

	//create a message queue/window which will respond to messages sent to it
	//RainWindow * which is returned must be freed
	RainWindow *createSendHandler(std::unordered_map<UINT, RainWindow::MSGFC> *msgm);
}