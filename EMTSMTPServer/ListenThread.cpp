#include "ListenThread.h"

namespace Mono3 {
	namespace SMTPServer {
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
			ltParam.recvParam.funcParam = NULL;

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

			//join the recvThread
			CancelSynchronousIo(ltParam.hRecvThread);
			WaitForSingleObject(ltParam.hRecvThread, 0);

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

			//wait for client to connect
			int error = Rain::servAcceptClient(ltParam.cSocket, *ltParam.lSocket);
			if (error == WSAEINTR) //the listening socket has been closed from the outside; we are terminating program soon
				return 0;
			else if (error) { //actual error
				Rain::reportError(error, "failed in ListenThread.cpp, at Rain::servAcceptClient");
				return -1;
			}

			//logging
			std::cout << Rain::getTime() << " Client connected from " << Rain::getClientNumIP(ltParam.cSocket) << "\r\n";
			Rain::fastOutputFile(ltParam.config->at("logFile"), Rain::getTime() + " Client connected from " + Rain::getClientNumIP(ltParam.cSocket) + "\r\n", true);
			
			//first, send the initial HELO
			Rain::sendText(ltParam.cSocket, ltParam.config->at("init220") + "\r\n");

			//set up the recvParam to pass to recvThread
			RecvThreadParam *rtParam = new RecvThreadParam();
			rtParam->pLTParam = &ltParam;

			ltParam.recvParam.socket = &ltParam.cSocket;
			ltParam.recvParam.message = &(rtParam->message); //store message in RecvThreadParam
			ltParam.recvParam.bufLen = Rain::strToT<std::size_t>(ltParam.config->at("recvBufferLength")); //recv buffer length
			ltParam.recvParam.funcParam = rtParam; //parameter to be passed to following functions
			ltParam.recvParam.onProcessMessage = onProcessMessage; //called when any message comes in
			ltParam.recvParam.onRecvInit = onRecvThreadInit; //called at the beginning of the recvThread, nothing for now
			ltParam.recvParam.onRecvEnd = onRecvThreadEnd; //called at the end of recvThread

														   //processing this socket will be handled by the recvThread
			ltParam.hRecvThread = CreateThread(NULL, 0, Rain::recvThread, reinterpret_cast<void *>(&ltParam.recvParam), NULL, NULL);

			//once we accept a client, create a new clientthread to listen for more connections, thus the linked list structure
			//before starting a new thread, create a new ListenThreadParam for that ListenThread
			ltParam.ltLLMutex->lock();

			ListenThreadParam *newLTParam = new ListenThreadParam();
			newLTParam->lSocket = ltParam.lSocket;
			newLTParam->ltLLMutex = ltParam.ltLLMutex;
			newLTParam->config = ltParam.config;

			//attach the listenThread directly after the current one in the linked list
			newLTParam->prevLTP = &ltParam;
			newLTParam->nextLTP = ltParam.nextLTP;
			ltParam.nextLTP = newLTParam;
			newLTParam->nextLTP->prevLTP = newLTParam;

			ltParam.ltLLMutex->unlock();

			CreateThread(NULL, 0, listenThread, reinterpret_cast<LPVOID>(newLTParam), 0, NULL);

			return 0;
		}
	}
}