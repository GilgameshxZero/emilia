/*
Standard
*/

#pragma once

#include <string>
#include <unordered_map>
#include <Windows.h>

namespace Rain {
	class RecvHandlerParam {
		public:
		//event handler function type
		typedef int(*EventHandler) (void *);

		RecvHandlerParam();
		RecvHandlerParam(SOCKET *socket, std::string *message, std::size_t bufLen, void *funcparam, EventHandler onProcessMessage, EventHandler onRecvInit, EventHandler onRecvExit);

		//socket between server and client
		SOCKET *socket;

		//place where message is stored for use by any EventHandler
		std::string *message;

		//length of buffer for recv
		std::size_t bufLen;

		//parameter to be passed to EventHandler
		void *funcParam;

		//called whenever recv returns something to the buffer
		//return nonzero to terminate recv loop
		EventHandler onProcessMessage;

		//called when RecvThread is about to start or end
		//return nonzero to terminate recv immediately
		EventHandler onRecvInit;
		EventHandler onRecvExit;
	};

	//calls a function whenever a message is received; 
	DWORD WINAPI recvThread(LPVOID lpParameter); //don't use this; use createRecvThread
	HANDLE createRecvThread(
		RecvHandlerParam *recvparam, //if NULL: returns pointer to RecvParam that must be freed when the thread ends
											//if not NULL: ignores the the next 6 parameters, and uses this as the param for recvThread
		SOCKET *connection = NULL,
		std::string *message = NULL, //where the message is stored each time OnProcessMessage is called
		int buflen = NULL, //the buffer size of the receive function
		void *funcparam = NULL, //additional parameter to pass to the functions OnProcessMessage and OnRecvEnd
		RecvHandlerParam::EventHandler OnProcessMessage = NULL,
		RecvHandlerParam::EventHandler OnRecvInit = NULL, //called at the beginning of thread
		RecvHandlerParam::EventHandler OnRecvEnd = NULL, //called when the other side shuts down send
		DWORD dwCreationFlags = 0,
		SIZE_T dwStackSize = 0,
		LPDWORD lpThreadId = NULL,
		LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL);
}