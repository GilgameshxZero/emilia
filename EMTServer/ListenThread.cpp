#include "ListenThread.h"

namespace Mono3 {
	DWORD WINAPI listenThread(LPVOID lpParameter) {
		ListenThreadParam &ltParam = *reinterpret_cast<ListenThreadParam *>(lpParameter);
		CTLLNode *thisnode;

		//make ctparam.end the node for the current thread
		ltParam.ltLLMutex->lock(); //lock the LL while making modifications
		if (ltParam.beg == NULL) //empty LL
		{
			ltParam.beg = ltParam.end = thisnode = new CTLLNode();
			ltParam.beg->prev = ltParam.beg->next = NULL;
		} else //append new CTLLNode to end of LL
		{
			ltParam.end->next = thisnode = new CTLLNode();
			ltParam.end->next->prev = ltParam.end;
			ltParam.end->next->next = NULL;
			ltParam.end = ltParam.end->next;
		}
		ltParam.ltLLMutex->unlock();

		//create message queue for listenThread for terminate messages
		Rain::RainWindow rainWnd;
		std::unordered_map<UINT, Rain::RainWindow::MSGFC> msgm;
		msgm.insert(std::make_pair(WM_RAINAVAILABLE, onListenThreadInit));
		msgm.insert(std::make_pair(WM_RAINAVAILABLE + 1, onListenThreadEnd));
		rainWnd.create(&msgm);
		thisnode->hwnd = rainWnd.hwnd;

		ClientProcParam cpparam;
		cpparam.listensock = ltParam.lSocket;
		cpparam.ctllnode = thisnode;
		cpparam.ctparam = &ltParam;
		cpparam.hrecvthread = NULL;
		cpparam.recvparam.funcparam = NULL;

		SetWindowLongPtr(thisnode->hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&cpparam));
		SendMessage(thisnode->hwnd, WM_RAINAVAILABLE, 0, 0); //update window and start accepting clients

		WPARAM wndReturn = rainWnd.enterMessageLoop();
		rainWnd.~RainWindow(); //double-check to make sure rainWnd is destroyed

		//remove this CTLLNode from the linked list
		ltParam.ltLLMutex->lock(); //lock the linked list while making modifications
		if (thisnode == ltParam.beg) {
			ltParam.beg = thisnode->next;
			if (ltParam.beg)
				ltParam.beg->prev = NULL;
		} else
			thisnode->prev->next = thisnode->next;
		if (thisnode == ltParam.end) {
			ltParam.end = thisnode->prev;
			if (ltParam.end)
				ltParam.end->next = NULL;
		} else
			thisnode->next->prev = thisnode->prev;
		delete thisnode;
		ltParam.ltLLMutex->unlock();

		return static_cast<DWORD>(wndReturn);
	}

	LRESULT onListenThreadEnd(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		ClientProcParam &cpparam = *reinterpret_cast<ClientProcParam *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		Rain::shutdownSocketSend(cpparam.clientsock);
		closesocket(cpparam.clientsock);
		if (cpparam.hrecvthread)
			CloseHandle(cpparam.hrecvthread);

		//to actually send WM_DESTROY from the current thread
		DestroyWindow(hWnd);

		//to terminate the message loop
		PostQuitMessage(0);

		return 0;
	}
	LRESULT onListenThreadInit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		ClientProcParam &cpparam = *reinterpret_cast<ClientProcParam *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		int error = Rain::servAcceptClient(cpparam.clientsock, *cpparam.listensock);
		if (error == WSAEINTR) //the listening socket has been closed from the outside; we are terminating program soon
			return 0;
		else if (error) { //actual error
			Rain::reportError(error, "failed in ListenThread.cpp, at Rain::servAcceptClient");
			return -1;
		}

		//std::cout << "Client accepted.\n";
		cpparam.recvparam.sock = &cpparam.clientsock;
		cpparam.recvparam.funcparam = reinterpret_cast<void *>(new RecvFuncParam());

		RecvFuncParam *rfparam = reinterpret_cast<RecvFuncParam *>(cpparam.recvparam.funcparam);
		rfparam->ctllnode = cpparam.ctllnode;
		rfparam->sock = &cpparam.clientsock;
		rfparam->waitingPOST = false;
		rfparam->serverRootDir = cpparam.ctparam->serverRootDir;
		rfparam->serverAux = cpparam.ctparam->serverAux;

		cpparam.recvparam.message = &(rfparam->message);
		cpparam.recvparam.buflen = 1024;
		cpparam.recvparam.OnProcessMessage = ProcClientMess; //called when any message comes in
		cpparam.recvparam.OnRecvInit = NULL;
		cpparam.recvparam.OnRecvEnd = OnClientRecvEnd;

		//processing this socket will be handed by the recvThread
		cpparam.hrecvthread = CreateThread(NULL, 0, Rain::recvThread, reinterpret_cast<void *>(&cpparam.recvparam), NULL, NULL);

		//once we accept a client, create a new clientthread to listen for more connections, thus the linked list structure
		CreateThread(NULL, 0, Mono3::listenThread, reinterpret_cast<LPVOID>(cpparam.ctparam), 0, NULL);

		return 0;
	}

	void OnClientRecvEnd(void *param) {
		//client wants to terminate socket
		RecvFuncParam &rfparam = *reinterpret_cast<RecvFuncParam *>(param);

		//use postmessage here because we want the thread of the window to process the message, allowing destroywindow to be called
		//WM_RAINAVAILABLE + 1 is the end message
		PostMessage(rfparam.ctllnode->hwnd, WM_RAINAVAILABLE + 1, 0, 0);

		//free WSA2RecvParam here, since recvThread won't need it anymore
		delete &rfparam;
	}
}