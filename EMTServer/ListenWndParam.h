#pragma once

#include "ListenThreadParam.h"

struct ClientProcParam {
	SOCKET *listensock, clientsock;
	Rain::WSA2RecvParam recvparam;
	HANDLE hrecvthread;
	ListenThreadParam *ctparam;
	CTLLNode *ctllnode;
};