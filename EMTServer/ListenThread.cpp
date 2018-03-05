#include "ListenThread.h"

namespace Mono3 {
	DWORD WINAPI listenThread(LPVOID lpParameter) {
		ListenThreadParam &ltParam = *reinterpret_cast<ListenThreadParam *>(lpParameter);
		
		//store this thread's handle in the ListenThreadParam
		ltParam.hThread = GetCurrentThread();

		//create message queue for listenThread for terminate messages
		std::unordered_map<UINT, Rain::RainWindow::MSGFC> msgm;
		msgm.insert(std::make_pair(WM_LISTENWNDINIT, onListenWndInit));
		msgm.insert(std::make_pair(WM_LISTENWNDEND, onListenWndEnd));
		ltParam.rainWnd.create(&msgm);

		ClientProcParam cpparam;
		cpparam.listensock = ltParam.lSocket;
		cpparam.ctparam = &ltParam;
		cpparam.hrecvthread = NULL;
		cpparam.recvparam.funcparam = NULL;

		SetWindowLongPtr(ltParam.rainWnd.hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&cpparam));
		SendMessage(ltParam.rainWnd.hwnd, WM_RAINAVAILABLE, 0, 0); //update window and start accepting clients

		WPARAM wndReturn = ltParam.rainWnd.enterMessageLoop();
		ltParam.rainWnd.~RainWindow(); //double-check to make sure rainWnd is destroyed

		//remove ltParam from the linked list structure
		ltParam.prevLTP->nextLTP = ltParam.nextLTP;
		ltParam.nextLTP->prevLTP = ltParam.prevLTP;

		//ltParam is no longer needed, free it here
		delete &ltParam;

		return static_cast<DWORD>(wndReturn);
	}

	LRESULT onListenWndEnd(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		ClientProcParam &cpparam = *reinterpret_cast<ClientProcParam *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		Rain::shutdownSocketSend(cpparam.clientsock);
		closesocket(cpparam.clientsock);
		if (cpparam.hrecvthread)
			CloseHandle(cpparam.hrecvthread);

		//do not free ListenThreadParam here! We should free it at the actual end of ListenThread

		//to actually send WM_DESTROY from the current thread
		DestroyWindow(hWnd);

		//to terminate the message loop
		PostQuitMessage(0);

		return 0;
	}
	LRESULT onListenWndInit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
		rfparam->ctllnode = cpparam.ctparam;
		rfparam->sock = &cpparam.clientsock;
		rfparam->waitingPOST = false;
		rfparam->serverRootDir = *cpparam.ctparam->serverRootDir;
		rfparam->serverAux = *cpparam.ctparam->serverAux;

		cpparam.recvparam.message = &(rfparam->message);
		cpparam.recvparam.buflen = 1024;
		cpparam.recvparam.OnProcessMessage = ProcClientMess; //called when any message comes in
		cpparam.recvparam.OnRecvInit = NULL;
		cpparam.recvparam.OnRecvEnd = OnClientRecvEnd;

		//processing this socket will be handled by the recvThread
		cpparam.hrecvthread = CreateThread(NULL, 0, Rain::recvThread, reinterpret_cast<void *>(&cpparam.recvparam), NULL, NULL);

		//once we accept a client, create a new clientthread to listen for more connections, thus the linked list structure
		//before starting a new thread, create a new ListenThreadParam for that ListenThread
		cpparam.ctparam->ltLLMutex->lock();

		ListenThreadParam *ltParam = new ListenThreadParam();
		ltParam->lSocket = cpparam.ctparam->lSocket;
		ltParam->ltLLMutex = cpparam.ctparam->ltLLMutex;
		ltParam->serverRootDir = cpparam.ctparam->serverRootDir;
		ltParam->serverAux = cpparam.ctparam->serverAux;

		//attach the listenThread directly after the current one in the linked list
		ltParam->prevLTP = cpparam.ctparam;
		ltParam->nextLTP = cpparam.ctparam->nextLTP;
		cpparam.ctparam->nextLTP = ltParam;
		ltParam->nextLTP->prevLTP = ltParam;

		cpparam.ctparam->ltLLMutex->unlock();

		CreateThread(NULL, 0, Mono3::listenThread, reinterpret_cast<LPVOID>(ltParam), 0, NULL);

		return 0;
	}

	void OnClientRecvEnd(void *param) {
		//client wants to terminate socket
		RecvFuncParam &rfparam = *reinterpret_cast<RecvFuncParam *>(param);

		//use postmessage here because we want the thread of the window to process the message, allowing destroywindow to be called
		//WM_RAINAVAILABLE + 1 is the end message
		PostMessage(rfparam.ctllnode->rainWnd.hwnd, WM_LISTENWNDEND, 0, 0);

		//free WSA2RecvParam here, since recvThread won't need it anymore
		delete &rfparam;
	}
}