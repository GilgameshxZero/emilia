#include "RainWSA2Server.h"

namespace Rain {
	HANDLE createListenThread(SOCKET &lSocket,
							  void *recvFuncParam,
							  std::size_t recvBufferLength,
							  Rain::WSA2RecvPMFunc onProcessMessage,
							  Rain::WSA2RecvInitFunc onRecvInit,
							  Rain::WSA2RecvExitFunc onRecvExit) {
		WSA2ListenThreadParam *ltParam = new WSA2ListenThreadParam();
		ltParam->lSocket = &lSocket;
		ltParam->recvFuncParam = recvFuncParam;
		ltParam->recvBufferLength = recvBufferLength;
		ltParam->onProcessMessage = onProcessMessage;
		ltParam->onRecvInit = onRecvInit;
		ltParam->onRecvExit = onRecvExit;
		return CreateThread(NULL, 0, listenThread, reinterpret_cast<LPVOID>(ltParam), 0, NULL);
	}

	DWORD WINAPI listenThread(LPVOID lpParameter) {
		WSA2ListenThreadParam &ltParam = *reinterpret_cast<WSA2ListenThreadParam *>(lpParameter); //specific to the listen thread

		//keep track of all WSA2ListenThreadRecvFuncParam spawned so that we can use them to end all recvThreads when thread needs to exit
		WSA2ListenThreadRecvFuncParam llHead, llTail;
		llHead.prevLTRFP = NULL;
		llHead.nextLTRFP = &llTail;
		llTail.prevLTRFP = &llHead;
		llTail.nextLTRFP = NULL;

		std::mutex llMutex;

		while (true) {
			//accept a socket
			SOCKET *cSocket = new SOCKET();

			int error = Rain::servAcceptClient(*cSocket, *ltParam.lSocket);
			if (error == WSAEINTR) {//listening closed from outside the thread; prepare to exit thread
				delete cSocket;
				break;
			}
			else if (error) { //unexpected
				Rain::reportError(error, "unexpected error in Rain::listenThread, at Rain::servAcceptClient");
				return -1;
			}

			//only when a client is accepted will we create new parameters and add them to the linked lists
			WSA2RecvFuncParam *rfParam = new WSA2RecvFuncParam();
			WSA2ListenThreadRecvFuncParam *ltrfParam = new WSA2ListenThreadRecvFuncParam();

			ltrfParam->llMutex = &llMutex;
			ltrfParam->pRFParam = rfParam;
			ltrfParam->ltrfdParam.callerParam = ltParam.recvFuncParam;
			ltrfParam->ltrfdParam.cSocket = cSocket;
			ltrfParam->pLTParam = &ltParam;
			ltrfParam->llMutex->lock();
			ltrfParam->prevLTRFP = llTail.prevLTRFP;
			ltrfParam->nextLTRFP = &llTail;
			ltrfParam->prevLTRFP->nextLTRFP = ltrfParam;
			llTail.prevLTRFP = ltrfParam;
			ltrfParam->llMutex->unlock();

			rfParam->bufLen = ltParam.recvBufferLength;
			rfParam->funcParam = ltrfParam;
			rfParam->message = &ltrfParam->ltrfdParam.message;
			rfParam->onProcessMessage = onListenThreadRecvProcessMessage;
			rfParam->onRecvInit = onListenThreadRecvInit;
			rfParam->onRecvExit = onListenThreadRecvExit;
			rfParam->socket = ltrfParam->ltrfdParam.cSocket;

			ltrfParam->hRecvThread = createRecvThread(rfParam);
		}

		//wait on all spawned and active recvThreads to exit
		while (llHead.nextLTRFP != &llTail) {
			//close client socket to force blocking WSA2 calls to finish
			llMutex.lock();
			Rain::shutdownSocketSend(*llHead.nextLTRFP->ltrfdParam.cSocket);
			closesocket(*llHead.nextLTRFP->ltrfdParam.cSocket);
			//in case thread gets shutdown while some operations are happening with its handle
			HANDLE curRecvThread = llHead.nextLTRFP->hRecvThread;
			llMutex.unlock();

			//join the recvThread
			CancelSynchronousIo(curRecvThread);
			WaitForSingleObject(curRecvThread, 0);
		}

		//free ltParam which was dynamically created by createListenThread
		delete &ltParam;

		return 0;
	}

	void onListenThreadRecvInit(void *funcParam) {
		//call delegate handler
		WSA2ListenThreadRecvFuncParam &ltrfParam = *reinterpret_cast<WSA2ListenThreadRecvFuncParam *>(funcParam);
		ltrfParam.pLTParam->onRecvInit(reinterpret_cast<void *>(&ltrfParam.ltrfdParam));
	}
	void onListenThreadRecvExit(void *funcParam) {
		WSA2ListenThreadRecvFuncParam &ltrfParam = *reinterpret_cast<WSA2ListenThreadRecvFuncParam *>(funcParam);

		//call delegate handler
		ltrfParam.pLTParam->onRecvExit(reinterpret_cast<void *>(&ltrfParam.ltrfdParam));

		//modify linked list and remove current ltrfParam
		ltrfParam.llMutex->lock();
		ltrfParam.prevLTRFP->nextLTRFP = ltrfParam.nextLTRFP;
		ltrfParam.nextLTRFP->prevLTRFP = ltrfParam.prevLTRFP;
		ltrfParam.llMutex->unlock();

		//free memory allocated in listenThread
		CloseHandle(ltrfParam.hRecvThread);
		delete ltrfParam.ltrfdParam.cSocket;
		delete ltrfParam.pRFParam;
		delete &ltrfParam;
	}
	int onListenThreadRecvProcessMessage(void *funcParam) {
		//call delegate handler
		WSA2ListenThreadRecvFuncParam &ltrfParam = *reinterpret_cast<WSA2ListenThreadRecvFuncParam *>(funcParam);
		return ltrfParam.pLTParam->onProcessMessage(reinterpret_cast<void *>(&ltrfParam.ltrfdParam));
	}
}