#include "network-recv-thread.h"

namespace Rain {
	DWORD WINAPI recvThread(LPVOID lpParameter) {
		RecvHandlerParam *recvparam = reinterpret_cast<RecvHandlerParam *>(lpParameter);
		char *buffer = new char[recvparam->bufLen];
		int ret;

		if (recvparam->onConnect != NULL)
			if (recvparam->onConnect(recvparam->funcParam))
				return 0;

		//receive data until the server closes the connection
		do {
			ret = recv(*recvparam->socket, buffer, static_cast<int>(recvparam->bufLen), 0);
			if (ret > 0) { //received ret bytes
				*recvparam->message = std::string(buffer, ret);
				if (recvparam->onMessage != NULL)
					if (recvparam->onMessage(recvparam->funcParam))
						break;
			} else if (ret == 0)
				break; //connection closed
			else { //failure
				ret = WSAGetLastError();
				break;
			}
		} while (ret > 0);

		delete[] buffer;
		if (recvparam->onDisconnect != NULL)
			recvparam->onDisconnect(recvparam->funcParam); //going to return soon regardless if nonzero

		return ret;
	}
	HANDLE createRecvThread(
		RecvHandlerParam *recvparam, //if NULL: returns pointer to RecvParam that must be freed when the thread ends
											//if not NULL: ignores the the next 6 parameters, and uses this as the param for recvThread
		SOCKET *connection,
		std::string *message, //where the message is stored each time onMessage is called
		int buflen, //the buffer size of the receive function
		void *funcParam, //additional parameter to pass to the functions onMessage and onDisconnect
		RecvHandlerParam::EventHandler onConnect, //called when thread starts
		RecvHandlerParam::EventHandler onMessage,
		RecvHandlerParam::EventHandler onDisconnect, //called when the other side shuts down send
		DWORD dwCreationFlags,
		SIZE_T dwStackSize,
		LPDWORD lpThreadId,
		LPSECURITY_ATTRIBUTES lpThreadAttributes) {
		if (recvparam == NULL)
			recvparam = new RecvHandlerParam(connection, message, buflen, funcParam, onConnect, onMessage, onDisconnect);

		std::thread newThread(recvThread, reinterpret_cast<LPVOID>(recvparam));
		newThread.detach();
		return newThread.native_handle();
	}
}