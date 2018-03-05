/*
Standard
*/

#pragma once

#include "RainWindow.h"
#include "RainWSA2Include.h"
#include "RainWSA2RecvParam.h"

#include <string>
#include <unordered_map>

namespace Rain {
	//send raw text over a socket
	int sendText(SOCKET &sock, const char *cstrtext, long long len);

	int sendHeader(SOCKET &sock, std::unordered_map<std::string, std::string> *headers);

	//calls a function whenever a message is received; 
	DWORD WINAPI recvThread(LPVOID lpParameter); //don't use this; use createRecvThread
	HANDLE createRecvThread(
		WSA2RecvParam *recvparam, //if NULL: returns pointer to RecvParam that must be freed when the thread ends
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