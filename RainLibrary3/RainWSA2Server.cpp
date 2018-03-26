#include "RainWSA2Server.h"

namespace Rain {
	HANDLE createListenThread(SOCKET lSocket,
							  std::size_t recvBufferLength,
							  void *recvFuncParam,
							  Rain::WSA2RecvPMFunc onProcessMessage,
							  Rain::WSA2RecvInitFunc onRecvInit,
							  Rain::WSA2RecvExitFunc onRecvExit) {
		CreateThread(NULL, 0, listenThread, reinterpret_cast<LPVOID>(ltParam), 0, NULL);
	}
}