#include "ListenThread.h"

namespace Mono3 {
	DWORD WINAPI listenThread(LPVOID lpParameter) {
		ListenThreadParam &ltParam = *reinterpret_cast<ListenThreadParam *>(lpParameter);
		
		//store this thread's handle in the ListenThreadParam
		//does not need to be close
		ltParam.hThread = GetCurrentThread();

		//create message queue for listenThread for terminate messages
		//one message queue/window per ListenThread
		//one ListenThreadParam per ListenThread and window
		std::unordered_map<UINT, Rain::RainWindow::MSGFC> msgm;
		msgm.insert(std::make_pair(WM_LISTENWNDINIT, onListenWndInit));
		msgm.insert(std::make_pair(WM_LISTENWNDEND, onListenWndEnd));
		ltParam.rainWnd.create(&msgm);

		//set some variables before starting the message queue/wnd, so that we know if they are used later
		ltParam.hRecvThread = NULL;
		ltParam.recvParam.funcparam = NULL;

		SetWindowLongPtr(ltParam.rainWnd.hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&ltParam));
		SendMessage(ltParam.rainWnd.hwnd, WM_LISTENWNDINIT, 0, 0); //update window and start accepting clients

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
		ListenThreadParam &ltParam = *reinterpret_cast<ListenThreadParam *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		//notify client that we are closing socket, then close it ourselves
		Rain::shutdownSocketSend(ltParam.cSocket);
		closesocket(ltParam.cSocket);

		//free up handle memory if it was used (i.e. a client was connected and an instance of RecvThread was started)
		if (ltParam.hRecvThread)
			CloseHandle(ltParam.hRecvThread);

		//do not free ListenThreadParam here! We should free it at the actual end of ListenThread

		//to actually send WM_DESTROY from the current thread
		DestroyWindow(hWnd);

		//to terminate the message loop
		PostQuitMessage(0);

		return 0;
	}
	LRESULT onListenWndInit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		ListenThreadParam &ltParam = *reinterpret_cast<ListenThreadParam *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		int error = Rain::servAcceptClient(ltParam.cSocket, *ltParam.lSocket);
		if (error == WSAEINTR) //the listening socket has been closed from the outside; we are terminating program soon
			return 0;
		else if (error) { //actual error
			Rain::reportError(error, "failed in ListenThread.cpp, at Rain::servAcceptClient");
			return -1;
		}

		//std::cout << "Client accepted.\n";
		ltParam.recvParam.sock = &ltParam.cSocket;
		ltParam.recvParam.funcparam = reinterpret_cast<void *>(new RecvFuncParam());

		RecvFuncParam *rfparam = reinterpret_cast<RecvFuncParam *>(ltParam.recvParam.funcparam);
		rfparam->ctllnode = &ltParam;
		rfparam->sock = &ltParam.cSocket;
		rfparam->waitingPOST = false;
		rfparam->serverRootDir = *ltParam.serverRootDir;
		rfparam->serverAux = *ltParam.serverAux;

		ltParam.recvParam.message = &(rfparam->message);
		ltParam.recvParam.buflen = 1024;
		ltParam.recvParam.OnProcessMessage = ProcClientMess; //called when any message comes in
		ltParam.recvParam.OnRecvInit = NULL;
		ltParam.recvParam.OnRecvEnd = OnClientRecvEnd;

		//processing this socket will be handled by the recvThread
		ltParam.hRecvThread = CreateThread(NULL, 0, Rain::recvThread, reinterpret_cast<void *>(&ltParam.recvParam), NULL, NULL);

		//once we accept a client, create a new clientthread to listen for more connections, thus the linked list structure
		//before starting a new thread, create a new ListenThreadParam for that ListenThread
		ltParam.ltLLMutex->lock();

		ListenThreadParam *newLTParam = new ListenThreadParam();
		newLTParam->lSocket = ltParam.lSocket;
		newLTParam->ltLLMutex = ltParam.ltLLMutex;
		newLTParam->serverRootDir = ltParam.serverRootDir;
		newLTParam->serverAux = ltParam.serverAux;

		//attach the listenThread directly after the current one in the linked list
		newLTParam->prevLTP = &ltParam;
		newLTParam->nextLTP = ltParam.nextLTP;
		ltParam.nextLTP = newLTParam;
		newLTParam->nextLTP->prevLTP = newLTParam;

		ltParam.ltLLMutex->unlock();

		CreateThread(NULL, 0, Mono3::listenThread, reinterpret_cast<LPVOID>(newLTParam), 0, NULL);

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