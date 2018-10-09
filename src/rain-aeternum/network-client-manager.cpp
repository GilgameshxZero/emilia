#include "network-client-manager.h"

namespace Rain {
	ClientSocketManager::ClientSocketManager() {
		this->socket = NULL;
		this->blockSendRawMessage = false;
		this->connectedPort = -1;
		this->socketStatus = -1;
		this->ipAddress = "";
		this->lowPort = this->highPort = 0;
		this->onConnectDelegate = this->onMessageDelegate = this->onDisconnectDelegate = NULL;
		this->msReconnectWaitMax = this->msSendWaitMax = 3000;
		this->recvBufLen = 65536;
		this->logger = NULL;

		this->hConnectThread = this->hSendThread = NULL;

		this->csmdhParam.csm = this;
		this->csmdhParam.delegateParam = NULL;
		this->csmdhParam.message = new std::string();

		this->connectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->messageDoneEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		this->recvExitComplete = CreateEvent(NULL, TRUE, TRUE, NULL);

		Rain::initWinsock22();
	}
	ClientSocketManager::~ClientSocketManager() {
		//shutdown send threads, if any
		this->clearMessageQueue();
		WaitForSingleObject(this->messageDoneEvent, INFINITE);

		//shutsdown connect threads, if any
		this->disconnectSocket();

		//critical: free addrs after shutting down connect threads, or there will be multithreading problems
		this->freePortAddrs();

		if (this->csmdhParam.message != NULL)
			delete this->csmdhParam.message;
		this->csmdhParam.message = NULL;
	}
	void ClientSocketManager::sendRawMessage(std::string request) {
		this->sendRawMessage(&request);
	}
	void ClientSocketManager::sendRawMessage(std::string *request) {
		//uses copy constructor
		this->messageQueue.push(*request);

		//if we are logging socket communications, do that here for outgoing communications
		if (this->logger != NULL)
			this->logger->logString(request);

		if (WaitForSingleObject(this->messageDoneEvent, 1) == WAIT_OBJECT_0) { //means that the object was set/the thread is not active currently
			//reset event outside of thread start so multiple calls to sendRawMessage won't create race conditions
			ResetEvent(this->messageDoneEvent);

			//start thread to send messages
			this->hSendThread = Rain::simpleCreateThread(ClientSocketManager::attemptSendMessageThread, this);
		}

		if (this->blockSendRawMessage)
			this->blockForMessageQueue(0);
	}
	void ClientSocketManager::clearMessageQueue() {
		this->messageQueue = std::queue<std::string>();
	}
	void ClientSocketManager::blockForMessageQueue(DWORD msTimeout) {
		WaitForSingleObject(this->messageDoneEvent, msTimeout);
	}
	bool ClientSocketManager::setSendRawMessageBlocking(bool blocking) {
		bool origValue = this->blockSendRawMessage;
		this->blockSendRawMessage = blocking;
		return origValue;
	}
	int ClientSocketManager::getSocketStatus() {
		return this->socketStatus;
	}
	void ClientSocketManager::blockForConnect(DWORD msTimeout) {
		WaitForSingleObject(this->connectEvent, msTimeout);
	}
	DWORD ClientSocketManager::getConnectedPort() {
		return this->connectedPort;
	}
	SOCKET &ClientSocketManager::getSocket() {
		return this->socket;
	}
	void ClientSocketManager::setClientTarget(std::string ipAddress, DWORD lowPort, DWORD highPort) {
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
			ResetEvent(this->connectEvent);
			this->hConnectThread = Rain::simpleCreateThread(ClientSocketManager::attemptConnectThread, this);
		}
	}
	void ClientSocketManager::setEventHandlers(RecvHandlerParam::EventHandler onConnect, RecvHandlerParam::EventHandler onMessage, RecvHandlerParam::EventHandler onDisconnect, void *funcParam) {
		this->onConnectDelegate = onConnect;
		this->onMessageDelegate = onMessage;
		this->onDisconnectDelegate = onDisconnect;
		this->csmdhParam.delegateParam = funcParam;
	}
	DWORD ClientSocketManager::setReconnectAttemptTime(DWORD msMaxInterval) {
		DWORD origValue = this->msReconnectWaitMax;
		this->msReconnectWaitMax = msMaxInterval;
		return origValue;
	}
	DWORD ClientSocketManager::setSendAttemptTime(DWORD msMaxInterval) {
		DWORD origValue = this->msSendWaitMax;
		this->msSendWaitMax = msMaxInterval;
		return origValue;
	}
	std::size_t ClientSocketManager::setRecvBufLen(std::size_t newLen) {
		std::size_t origValue = this->recvBufLen;
		this->recvBufLen = newLen;
		return origValue;
	}
	bool ClientSocketManager::setLogging(void *logger) {
		bool ret = (this->logger != NULL);
		this->logger = reinterpret_cast<LogStream *>(logger);
		return ret;
	}
	void ClientSocketManager::disconnectSocket() {
		if (this->socketStatus == this->STATUS_DISCONNECTED) {
			return;
		} else if (this->socketStatus == this->STATUS_CONNECTED || this->socketStatus == this->STATUS_CONNECTING) { //connected, so shutdown immediately or terminate thread on next check
			Rain::shutdownSocketSend(this->socket);
			closesocket(this->socket);

			//this will exit the connect thread
			this->socketStatus = this->STATUS_DISCONNECTED; //manually disconnect, so that we won't try to disconnect
			WaitForSingleObject(this->connectEvent, INFINITE);

			this->connectedPort = -1;

			//wait for onDisconnect if socket was connected before
			WaitForSingleObject(this->recvExitComplete, INFINITE);
		}
	}
	void ClientSocketManager::freePortAddrs() {
		for (std::size_t a = 0; a < this->portAddrs.size(); a++) {
			Rain::freeAddrInfo(&portAddrs[a]);
		}
		this->portAddrs.clear();
	}
	DWORD WINAPI ClientSocketManager::attemptConnectThread(LPVOID param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		//resetting the connect event should be placed outside this thread, before it is created, to make sure that any calls to the thread are waited upon as intended

		csm.msReconnectWait = 1;
		csm.freePortAddrs();
		csm.portAddrs.resize(csm.highPort - csm.lowPort + 1, NULL);

		Rain::createSocket(csm.socket);
		
		while (csm.socketStatus == csm.STATUS_CONNECTING) {
			Sleep(csm.msReconnectWait);
			if (csm.msReconnectWait < csm.msReconnectWaitMax)
				csm.msReconnectWait = min(csm.msReconnectWait * 2, csm.msReconnectWaitMax);

			//try to reconnect; assume ipAddress and port ranges are valid
			//if address has not yet been retreived, try to get it here
			for (DWORD a = csm.lowPort; a <= csm.highPort; a++) {
				if (csm.socketStatus != csm.STATUS_CONNECTING) //make sure status hasn't changed externally
					break;

				if (csm.portAddrs[a - csm.lowPort] == NULL) { //address not yet found, get it now
					if (Rain::getTargetAddr(&csm.portAddrs[a - csm.lowPort], csm.ipAddress, Rain::tToStr(a))) {
						Rain::reportError(WSAGetLastError(), "getaddrinfo error while connecting ClientSocketManager");
						continue;
					}
				}

				//any socket that is connected will pass through this step
				if (!Rain::connectTarget(csm.socket, &csm.portAddrs[a - csm.lowPort])) {
					csm.connectedPort = a;
					csm.socketStatus = csm.STATUS_CONNECTED;

					//attach the socket to a recvThread
					csm.rParam.bufLen = csm.recvBufLen;
					csm.rParam.funcParam = static_cast<void *>(&csm);
					csm.rParam.message = csm.csmdhParam.message;
					csm.rParam.onMessage = csm.onMessage;
					csm.rParam.onConnect = csm.onConnect;
					csm.rParam.onDisconnect = csm.onDisconnect;
					csm.rParam.socket = &csm.socket;
					Rain::createRecvThread(&csm.rParam);

					break;
				}
			}
		}

		CloseHandle(csm.hConnectThread);

		//this event signals that the thread has exited; check status to see if successful
		SetEvent(csm.connectEvent);
		return 0;
	}
	DWORD WINAPI ClientSocketManager::attemptSendMessageThread(LPVOID param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		csm.msSendMessageWait = 1;

		//attempt to send messages until csm.messageQueue is empty, at which point set the messageDoneEvent
		while (!csm.messageQueue.empty()) {
			csm.blockForConnect(csm.msSendMessageWait);

			//deal with differently depending on whether we connected on that block or not
			if (csm.socketStatus != csm.STATUS_CONNECTED &&
				csm.msSendMessageWait < csm.msSendWaitMax)
				csm.msSendMessageWait = min(csm.msSendMessageWait * 2, csm.msSendWaitMax);
			else if (csm.socketStatus == csm.STATUS_CONNECTED) {
				Rain::sendRawMessage(csm.socket, &csm.messageQueue.front());
				csm.messageQueue.pop();
			}
		}

		CloseHandle(csm.hSendThread);

		SetEvent(csm.messageDoneEvent);
		return 0;
	}
	int ClientSocketManager::onConnect(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);
		
		//reset an event for listeners of the end of onDisconnect
		ResetEvent(csm.recvExitComplete);

		return csm.onConnectDelegate == NULL ? 0 : csm.onConnectDelegate(&csm.csmdhParam);
	}
	int ClientSocketManager::onMessage(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		//if we are logging socket communications, do that here for incoming messages
		if (csm.logger != NULL)
			csm.logger->logString(csm.rParam.message);

		return csm.onMessageDelegate == NULL ? 0 : csm.onMessageDelegate(&csm.csmdhParam);
	}
	int ClientSocketManager::onDisconnect(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		//retry to connect, if original status was connected
		//if original status was disconnected, then don't reconnect
		if (csm.socketStatus == csm.STATUS_CONNECTED) {
			csm.socketStatus = csm.STATUS_CONNECTING;

			//create thread to attempt connect
			//thread exits when connect success
			ResetEvent(csm.connectEvent);
			csm.hConnectThread = Rain::simpleCreateThread(ClientSocketManager::attemptConnectThread, &csm);
		}

		//call delegate if it exists
		int ret = csm.onDisconnectDelegate == NULL ? 0 : csm.onDisconnectDelegate(&csm.csmdhParam);

		//set an event to listeners, that this function has been called
		SetEvent(csm.recvExitComplete);

		return ret;
	}
}