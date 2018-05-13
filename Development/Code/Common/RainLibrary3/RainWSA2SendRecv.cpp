#include "RainWSA2SendRecv.h"

namespace Rain {
	WSA2RecvFuncParam::WSA2RecvFuncParam() {
	}

	WSA2RecvFuncParam::WSA2RecvFuncParam(SOCKET *socket, std::string *message, int buflen, void *funcParam, WSA2RecvPMFunc onProcessMessage, WSA2RecvInitFunc onRecvInit, WSA2RecvExitFunc onRecvExit) {
		this->socket = socket;
		this->message = message;
		this->bufLen = bufLen;
		this->funcParam = funcParam;
		this->onProcessMessage = onProcessMessage;
		this->onRecvInit = onRecvInit;
		this->onRecvExit = onRecvExit;
	}

	int sendText(SOCKET &sock, const char *cstrtext, long long len) {
		long long sent = 0;
		int ret;

		while (sent < len) {
			ret = send(sock, cstrtext + sent, static_cast<int>(len - sent), 0);
			if (ret == SOCKET_ERROR) {
				ret = WSAGetLastError();
				return ret;
			}

			sent += ret;
		}

		return 0;
	}
	int sendText(SOCKET &sock, std::string strText) {
		return sendText(sock, strText.c_str(), strText.length());
	}

	int sendHeader(SOCKET &sock, std::unordered_map<std::string, std::string> *headers) {
		std::string message;
		for (std::unordered_map<std::string, std::string>::iterator it = headers->begin(); it != headers->end(); it++)
			message += it->first + ": " + it->second + "\n";
		message += "\n";
		return Rain::sendText(sock, message.c_str(), message.length());
	}

	DWORD WINAPI recvThread(LPVOID lpParameter) {
		WSA2RecvFuncParam *recvparam = reinterpret_cast<WSA2RecvFuncParam *>(lpParameter);
		char *buffer = new char[recvparam->bufLen];
		int ret;

		if (recvparam->onRecvInit != NULL)
			recvparam->onRecvInit(recvparam->funcParam);

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
			recvparam->onRecvExit(recvparam->funcParam);

		return ret;
	}

	HANDLE createRecvThread(
		WSA2RecvFuncParam *recvparam, //if NULL: returns pointer to RecvParam that must be freed when the thread ends
								 //if not NULL: ignores the the next 6 parameters, and uses this as the param for recvThread
		SOCKET *connection,
		std::string *message, //where the message is stored each time OnProcessMessage is called
		int buflen, //the buffer size of the receive function
		void *funcparam, //additional parameter to pass to the functions OnProcessMessage and OnRecvEnd
		WSA2RecvPMFunc OnProcessMessage,
		WSA2RecvInitFunc OnRecvInit, //called when thread starts
		WSA2RecvExitFunc OnRecvEnd, //called when the other side shuts down send
		DWORD dwCreationFlags,
		SIZE_T dwStackSize,
		LPDWORD lpThreadId,
		LPSECURITY_ATTRIBUTES lpThreadAttributes) {
		if (recvparam == NULL)
			recvparam = new WSA2RecvFuncParam(connection, message, buflen, funcparam, OnProcessMessage, OnRecvInit, OnRecvEnd);

		return CreateThread(lpThreadAttributes, dwStackSize, recvThread, reinterpret_cast<LPVOID>(recvparam), dwCreationFlags, lpThreadId);
	}

	RainWindow *createSendHandler(std::unordered_map<UINT, RainWindow::MSGFC> *msgm) {
		RainWindow *rw = new RainWindow();
		rw->create(msgm, NULL, NULL, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, "", NULL, NULL, "", WS_POPUP, 0, 0, 0, 0, NULL, NULL, RainWindow::NULLCLASSNAME);

		return rw;
	}
}