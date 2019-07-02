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

		this->retryOnDisconnect = false;

		this->hConnectThread = this->hSendThread = NULL;

		this->csmdhParam.csm = this;
		this->csmdhParam.delegateParam = NULL;
		this->csmdhParam.message = new std::string();

		this->connectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->messageDoneEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		this->messageToSendEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->recvExitComplete = CreateEvent(NULL, TRUE, TRUE, NULL);

		this->destructing = false;

		int error = initWinsock22();
		if (error) {
			reportError(error, "ClientSocketManager: initWinsock22 failed, no resolution implemented");
		}

		//create the send thread once for every manager
		this->hSendThread = simpleCreateThread(ClientSocketManager::attemptSendMessageThread, this);
	}
	ClientSocketManager::~ClientSocketManager() {
		//shutdown send threads, if any
		this->destructing = true;
		this->clearMessageQueue();
		ResetEvent(this->messageDoneEvent);
		SetEvent(this->messageToSendEvent);
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
		//reset event outside of thread start so multiple calls to sendRawMessage won't create race conditions
		this->queueMutex.lock();
		if (this->messageQueue.size() == 0) {
			ResetEvent(this->messageDoneEvent);
			SetEvent(this->messageToSendEvent);
		}
		this->messageQueue.push(*request);
		this->queueMutex.unlock();

		//if we are logging socket communications, do that here for outgoing communications
		if (this->logger != NULL)
			this->logger->logString(request);
		if (this->blockSendRawMessage)
			this->blockForMessageQueue(0);
	}
	void ClientSocketManager::clearMessageQueue() {
		this->queueMutex.lock();
		this->messageQueue = std::queue<std::string>();
		this->queueMutex.unlock();
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
	std::string ClientSocketManager::getTargetIP() {
		return this->ipAddress;
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
			this->hConnectThread = simpleCreateThread(ClientSocketManager::attemptConnectThread, this);
		}
	}
	void ClientSocketManager::retryConnect() {
		if (this->socketStatus == this->STATUS_DISCONNECTED) {
			//create thread to attempt connect
			//thread exits when connect success
			ResetEvent(this->connectEvent);
			this->hConnectThread = simpleCreateThread(ClientSocketManager::attemptConnectThread, this);
		}
	}
	bool ClientSocketManager::setRetryOnDisconnect(bool value) {
		bool orig = this->retryOnDisconnect;
		this->retryOnDisconnect = value;
		return orig;
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
			shutdownSocketSend(this->socket);
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
			freeAddrInfo(&portAddrs[a]);
		}
		this->portAddrs.clear();
	}
	DWORD WINAPI ClientSocketManager::attemptConnectThread(LPVOID param) {
		ClientSocketManager &csm = *reinterpret_cast<ClientSocketManager *>(param);

		//resetting the connect event should be placed outside this thread, before it is created, to make sure that any calls to the thread are waited upon as intended

		csm.msReconnectWait = 1;
		csm.freePortAddrs();
		csm.portAddrs.resize(csm.highPort - csm.lowPort + 1, NULL);

		if (createSocket(csm.socket)) {
			reportError(WSAGetLastError(), "ClientSocketManager: createSocket failed, aborting...");
			SetEvent(csm.connectEvent);
			return -1;
		}

		while (csm.socketStatus == csm.STATUS_CONNECTING) {
			Rain::sleep(csm.msReconnectWait);
			if (csm.msReconnectWait < csm.msReconnectWaitMax)
				csm.msReconnectWait = min(csm.msReconnectWait * 2, csm.msReconnectWaitMax);

			//try to reconnect; assume ipAddress and port ranges are valid
			//if address has not yet been retreived, try to get it here
			for (DWORD a = csm.lowPort; a <= csm.highPort; a++) {
				if (csm.socketStatus != csm.STATUS_CONNECTING) //make sure status hasn't changed externally
					break;

				if (csm.portAddrs[a - csm.lowPort] == NULL) { //address not yet found, get it now
					if (getTargetAddr(&csm.portAddrs[a - csm.lowPort], csm.ipAddress, Rain::tToStr(a))) {
						reportError(WSAGetLastError(), "ClientSocketManager: getTargetAddr failed, retrying...");
						continue;
					}
				}

				//any socket that is connected will pass through this step
				if (!connectTarget(csm.socket, &csm.portAddrs[a - csm.lowPort])) {
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
					createRecvThread(&csm.rParam);

					break;
				} else {
					reportError(WSAGetLastError(), "ClientSocketManager: connectTarget failed, retrying...");
					std::cerr << csm.socket << Rain::CRLF;
					continue;
				}
			}
		}

		//this event signals that the thread has exited; check status to see if successful
		SetEvent(csm.connectEvent);
		return 0;
	}
	DWORD WINAPI ClientSocketManager::attemptSendMessageThread(LPVOID param) {
		ClientSocketManager &csm = *reinterpret_cast<ClientSocketManager *>(param);

		while (!csm.destructing) {
			//wait until we know there are messages in queue
			WaitForSingleObject(csm.messageToSendEvent, INFINITE);

			csm.msSendMessageWait = 1;

			//attempt to send messages until csm.messageQueue is empty, at which point set the messageDoneEvent
			csm.queueMutex.lock();
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
				csm.queueMutex.unlock();
				//unlock temporarily to allow other functions to maybe continue
				csm.queueMutex.lock();
			}
			ResetEvent(csm.messageToSendEvent);
			csm.queueMutex.unlock();
			SetEvent(csm.messageDoneEvent);
		}

		return 0;
	}
	int ClientSocketManager::onConnect(void *param) {
		ClientSocketManager &csm = *reinterpret_cast<ClientSocketManager *>(param);

		//reset an event for listeners of the end of onDisconnect
		ResetEvent(csm.recvExitComplete);

		return csm.onConnectDelegate == NULL ? 0 : csm.onConnectDelegate(&csm.csmdhParam);
	}
	int ClientSocketManager::onMessage(void *param) {
		ClientSocketManager &csm = *reinterpret_cast<ClientSocketManager *>(param);

		//if we are logging socket communications, do that here for incoming messages
		if (csm.logger != NULL)
			csm.logger->logString(csm.rParam.message);

		return csm.onMessageDelegate == NULL ? 0 : csm.onMessageDelegate(&csm.csmdhParam);
	}
	int ClientSocketManager::onDisconnect(void *param) {
		ClientSocketManager &csm = *reinterpret_cast<ClientSocketManager *>(param);

		csm.socketStatus = csm.STATUS_DISCONNECTED;

		//call delegate if it exists
		int ret = csm.onDisconnectDelegate == NULL ? 0 : csm.onDisconnectDelegate(&csm.csmdhParam);

		//set an event to listeners, that this function has been called
		SetEvent(csm.recvExitComplete);

		//if set, we want to try to reconnect
		if (csm.retryOnDisconnect) {
			csm.socketStatus = csm.STATUS_CONNECTING;

			//create thread to attempt connect
			//thread exits when connect success
			ResetEvent(csm.connectEvent);
			csm.hConnectThread = simpleCreateThread(ClientSocketManager::attemptConnectThread, &csm);
		}

		return ret;
	}
}