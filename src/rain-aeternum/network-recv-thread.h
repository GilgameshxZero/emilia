/*
Standard
*/

#pragma once

#include "network-recv-param.h"

#include <string>
#include <thread>
#include <unordered_map>
#include <Windows.h>

namespace Rain {
	//calls a function whenever a message is received; 
	DWORD WINAPI recvThread(LPVOID lpParameter); //don't use this; use createRecvThread
	HANDLE createRecvThread(
		RecvHandlerParam *recvparam, //if NULL: returns pointer to RecvParam that must be freed when the thread ends
											//if not NULL: ignores the the next 6 parameters, and uses this as the param for recvThread
		SOCKET *connection = NULL,
		std::string *message = NULL, //where the message is stored each time onMessage is called
		int buflen = NULL, //the buffer size of the receive function
		void *funcParam = NULL, //additional parameter to pass to the functions onMessage and onDisconnect
		RecvHandlerParam::EventHandler onConnect = NULL, //called at the beginning of thread
		RecvHandlerParam::EventHandler onMessage = NULL,
		RecvHandlerParam::EventHandler onDisconnect = NULL, //called when the other side shuts down send
		DWORD dwCreationFlags = 0,
		SIZE_T dwStackSize = 0,
		LPDWORD lpThreadId = NULL,
		LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL);
}