#include "NetworkServerManager.h"

namespace Rain {
	ServerSocketManager::ServerSocketManager() {
		this->socket = NULL;
		this->blockSendRawMessage = false;
		this->socketStatus = -1;
		this->ipAddress = "";
		this->lowPort = this->highPort = 0;
		this->onConnect = this->onMessage = this->onDisconnect = NULL;
		this->funcParam = NULL;
		this->msReconnectWaitMax = this->msSendWaitMax = 3000;
		this->recvBufLen = 65536;
		this->logger = NULL;

		this->connectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->messageDoneEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

		Rain::initWinsock22();
	}
	ServerSocketManager::~ServerSocketManager() {
		CloseHandle(this->connectEvent);

		//this automatically closes the send thread
		CloseHandle(this->messageDoneEvent);

		//shutsdown connect threads, if any
		this->socketStatus = this->STATUS_DISCONNECTED;

		Rain::shutdownSocketSend(this->socket);
		closesocket(this->socket);
	}
	void ServerSocketManager::sendRawMessage(std::string request) {
		this->sendRawMessage(&request);
	}
	void ServerSocketManager::sendRawMessage(std::string *request) {
		//uses copy constructor
		this->messageQueue.push(*request);

		//if we are logging socket communications, do that here for outgoing communications
		if (this->logger != NULL)
			this->logger->logString(request);

		if (WaitForSingleObject(this->messageDoneEvent, 1) == WAIT_OBJECT_0) { //means that the object was set/the thread is not active currently
																			   //reset event outside of thread start so multiple calls to sendRawMessage won't create race conditions
			ResetEvent(this->messageDoneEvent);

			//start thread to send messages
			Rain::simpleCreateThread(ClientSocketManager::attemptSendMessageThread, this);
		}

		if (this->blockSendRawMessage)
			this->blockForMessageQueue(0);
	}
	SOCKET &ServerSocketManager::getSocket() {
		return this->socket;
	}
	void ServerSocketManager::setEventHandlers(RecvHandlerParam::EventHandler onConnect, RecvHandlerParam::EventHandler onMessage, RecvHandlerParam::EventHandler onDisconnect, void *funcParam) {
		this->onConnect = onConnect;
		this->onMessage = onMessage;
		this->onDisconnect = onDisconnect;
		this->funcParam = funcParam;
	}
	bool ServerSocketManager::setLogging(bool enable, void *logger) {
		bool ret = (this->logger != NULL);
		this->logger = reinterpret_cast<RainLogger *>(logger);
		return ret;
	}
	int ServerSocketManager::onRecvInit(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		csm.rParam.message = new std::string();
		return csm.onConnect == NULL ? 0 : csm.onConnect(csm.funcParam);
	}
	int ServerSocketManager::onRecvExit(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		//retry to connect
		csm.socketStatus = csm.STATUS_CONNECTING;

		//create thread to attempt connect
		//thread exits when connect success
		Rain::simpleCreateThread(ClientSocketManager::attemptConnectThread, &csm);

		delete csm.rParam.message;
		return csm.onDisconnect == NULL ? 0 : csm.onDisconnect(csm.funcParam);
	}
	int ServerSocketManager::onProcessMessage(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		//if we are logging socket communications, do that here for incoming messages
		if (csm.logger != NULL)
			csm.logger->logString(csm.rParam.message);

		return csm.onMessage == NULL ? 0 : csm.onMessage(csm.funcParam);
	}

	ServerManager::ServerManager() {
		this->socket = NULL;
		this->blockSendRawMessage = false;
		this->socketStatus = -1;
		this->ipAddress = "";
		this->lowPort = this->highPort = 0;
		this->onConnect = this->onMessage = this->onDisconnect = NULL;
		this->funcParam = NULL;
		this->msReconnectWaitMax = this->msSendWaitMax = 3000;
		this->recvBufLen = 65536;
		this->logger = NULL;

		this->connectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->messageDoneEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

		Rain::initWinsock22();
	}
	ServerManager::~ServerManager() {
		CloseHandle(this->connectEvent);

		//this automatically closes the send thread
		CloseHandle(this->messageDoneEvent);

		//shutsdown connect threads, if any
		this->socketStatus = this->STATUS_DISCONNECTED;

		Rain::shutdownSocketSend(this->socket);
		closesocket(this->socket);
	}
	SOCKET &ServerManager::getSocket() {
		return this->socket;
	}
	void ServerManager::setServerListen(DWORD lowPort, DWORD highPort) {
		this->disconnectSocket();
		this->ipAddress = ipAddress;

		if (this->ipAddress.length() == 0) {
			this->lowPort = this->highPort = 0;
		} else {
			this->lowPort = lowPort;
			this->highPort = highPort;

			this->socketStatus = this->STATUS_CONNECTING;

			//create thread to attempt connect
			//thread exits when connect success
			Rain::simpleCreateThread(ClientSocketManager::attemptConnectThread, this);
		}
	}
	void ServerManager::setEventHandlers(RecvHandlerParam::EventHandler onConnect, RecvHandlerParam::EventHandler onMessage, RecvHandlerParam::EventHandler onDisconnect, void *funcParam) {
		this->onConnect = onConnect;
		this->onMessage = onMessage;
		this->onDisconnect = onDisconnect;
		this->funcParam = funcParam;
	}
	std::size_t ServerManager::setRecvBufLen(std::size_t newLen) {
		std::size_t origValue = this->recvBufLen;
		this->recvBufLen = newLen;
		return origValue;
	}
	int ServerManager::onRecvInit(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		csm.rParam.message = new std::string();
		return csm.onConnect == NULL ? 0 : csm.onConnect(csm.funcParam);
	}
	int ServerManager::onRecvExit(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		//retry to connect
		csm.socketStatus = csm.STATUS_CONNECTING;

		//create thread to attempt connect
		//thread exits when connect success
		Rain::simpleCreateThread(ClientSocketManager::attemptConnectThread, &csm);

		delete csm.rParam.message;
		return csm.onDisconnect == NULL ? 0 : csm.onDisconnect(csm.funcParam);
	}
	int ServerManager::onProcessMessage(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		//if we are logging socket communications, do that here for incoming messages
		if (csm.logger != NULL)
			csm.logger->logString(csm.rParam.message);

		return csm.onMessage == NULL ? 0 : csm.onMessage(csm.funcParam);
	}

	HANDLE createListenThread(SOCKET &lSocket,
							  void *recvFuncParam,
							  std::size_t recvBufferLength,
							  Rain::RecvHandlerParam::EventHandler onProcessMessage,
							  Rain::RecvHandlerParam::EventHandler onRecvInit,
							  Rain::RecvHandlerParam::EventHandler onRecvExit) {
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

			*ltParam.lSocket = Rain::acceptClientSocket(*cSocket);
			int error = WSAGetLastError();
			if (error == WSAEINTR) {//listening closed from outside the thread; prepare to exit thread
				delete cSocket;
				break;
			}
			else if (error) { //unexpected
				Rain::reportError(error, "unexpected error in Rain::listenThread, at Rain::servAcceptClient");
				return -1;
			}

			//only when a client is accepted will we create new parameters and add them to the linked lists
			RecvHandlerParam *rfParam = new RecvHandlerParam();
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

	int onListenThreadRecvInit(void *funcParam) {
		//call delegate handler
		WSA2ListenThreadRecvFuncParam &ltrfParam = *reinterpret_cast<WSA2ListenThreadRecvFuncParam *>(funcParam);
		return ltrfParam.pLTParam->onRecvInit(reinterpret_cast<void *>(&ltrfParam.ltrfdParam));
	}
	int onListenThreadRecvExit(void *funcParam) {
		WSA2ListenThreadRecvFuncParam &ltrfParam = *reinterpret_cast<WSA2ListenThreadRecvFuncParam *>(funcParam);

		//call delegate handler
		int delRtrn = ltrfParam.pLTParam->onRecvExit(reinterpret_cast<void *>(&ltrfParam.ltrfdParam));

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

		return delRtrn;
	}
	int onListenThreadRecvProcessMessage(void *funcParam) {
		//call delegate handler
		WSA2ListenThreadRecvFuncParam &ltrfParam = *reinterpret_cast<WSA2ListenThreadRecvFuncParam *>(funcParam);
		return ltrfParam.pLTParam->onProcessMessage(reinterpret_cast<void *>(&ltrfParam.ltrfdParam));
	}
}