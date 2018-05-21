/*
Standard
*/

#pragma once

#include "NetworkRecvHandlerParam.h"

#include <string>
#include <unordered_map>

namespace Rain {
	//calls a function whenever a message is received; 
	DWORD WINAPI recvThread(LPVOID lpParameter); //don't use this; use createRecvThread
	HANDLE createRecvThread(
		NetworkRecvHandlerParam *recvparam, //if NULL: returns pointer to RecvParam that must be freed when the thread ends
											//if not NULL: ignores the the next 6 parameters, and uses this as the param for recvThread
		SOCKET *connection = NULL,
		std::string *message = NULL, //where the message is stored each time OnProcessMessage is called
		int buflen = NULL, //the buffer size of the receive function
		void *funcparam = NULL, //additional parameter to pass to the functions OnProcessMessage and OnRecvEnd
		NetworkRecvHandlerParam::EventHandler OnProcessMessage = NULL,
		NetworkRecvHandlerParam::EventHandler OnRecvInit = NULL, //called at the beginning of thread
		NetworkRecvHandlerParam::EventHandler OnRecvEnd = NULL, //called when the other side shuts down send
		DWORD dwCreationFlags = 0,
		SIZE_T dwStackSize = 0,
		LPDWORD lpThreadId = NULL,
		LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL);
}