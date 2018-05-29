#include "NetworkRecvThread.h"

namespace Rain {
	RecvHandlerParam::RecvHandlerParam() {
	}

	RecvHandlerParam::RecvHandlerParam(SOCKET *socket, std::string *message, std::size_t buflen, void *funcParam, EventHandler onProcessMessage, EventHandler onRecvInit, EventHandler onRecvExit) {
		this->socket = socket;
		this->message = message;
		this->bufLen = bufLen;
		this->funcParam = funcParam;
		this->onProcessMessage = onProcessMessage;
		this->onRecvInit = onRecvInit;
		this->onRecvExit = onRecvExit;
	}

	DWORD WINAPI recvThread(LPVOID lpParameter) {
		RecvHandlerParam *recvparam = reinterpret_cast<RecvHandlerParam *>(lpParameter);
		char *buffer = new char[recvparam->bufLen];
		int ret;

		if (recvparam->onRecvInit != NULL)
			if (recvparam->onRecvInit(recvparam->funcParam))
				return 0;

		//receive data until the server closes the connection
		do {
			ret = recv(*recvparam->socket, buffer, static_cast<int>(recvparam->bufLen), 0);
			if (ret > 0) //received ret bytes
			{
				*recvparam->message = std::string(buffer, ret);
				if (recvparam->onProcessMessage != NULL)
					if (recvparam->onProcessMessage(recvparam->funcParam))
						break;
			} else if (ret == 0)
				break; //connection closed
			else //failure
			{
				ret = WSAGetLastError();
				break;
			}
		} while (ret > 0);

		delete[] buffer;
		if (recvparam->onRecvExit != NULL)
			recvparam->onRecvExit(recvparam->funcParam); //going to return soon regardless if nonzero

		return ret;
	}
	HANDLE createRecvThread(
		RecvHandlerParam *recvparam, //if NULL: returns pointer to RecvParam that must be freed when the thread ends
											//if not NULL: ignores the the next 6 parameters, and uses this as the param for recvThread
		SOCKET *connection,
		std::string *message, //where the message is stored each time OnProcessMessage is called
		int buflen, //the buffer size of the receive function
		void *funcparam, //additional parameter to pass to the functions OnProcessMessage and OnRecvEnd
		RecvHandlerParam::EventHandler OnProcessMessage,
		RecvHandlerParam::EventHandler OnRecvInit, //called when thread starts
		RecvHandlerParam::EventHandler OnRecvEnd, //called when the other side shuts down send
		DWORD dwCreationFlags,
		SIZE_T dwStackSize,
		LPDWORD lpThreadId,
		LPSECURITY_ATTRIBUTES lpThreadAttributes) {
		if (recvparam == NULL)
			recvparam = new RecvHandlerParam(connection, message, buflen, funcparam, OnProcessMessage, OnRecvInit, OnRecvEnd);

		return CreateThread(lpThreadAttributes, dwStackSize, recvThread, reinterpret_cast<LPVOID>(recvparam), dwCreationFlags, lpThreadId);
	}
}