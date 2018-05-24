#include "NetworkClientManager.h"

namespace Rain {
	ClientSocketManager::ClientSocketManager() {
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
	ClientSocketManager::~ClientSocketManager() {
		CloseHandle(this->connectEvent);

		//this automatically closes the send thread
		CloseHandle(this->messageDoneEvent);

		//shutsdown connect threads, if any
		this->socketStatus = this->STATUS_DISCONNECTED;

		Rain::shutdownSocketSend(this->socket);
		closesocket(this->socket);
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
			Rain::simpleCreateThread(ClientSocketManager::attemptSendMessageThread, this);
		}

		if (this->blockSendRawMessage)
			this->blockForMessageQueue(0);
	}
	void ClientSocketManager::clearMessageQueue() {
		this->messageQueue = std::queue<std::string>();
	}
	void ClientSocketManager::blockForMessageQueue(DWORD msTimeout) {
		WaitForSingleObject(this->messageDoneEvent, msTimeout == 0 ? INFINITE : msTimeout);
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
		WaitForSingleObject(this->connectEvent, msTimeout == 0 ? INFINITE : msTimeout);
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
			Rain::simpleCreateThread(ClientSocketManager::attemptConnectThread, this);
		}
	}
	void ClientSocketManager::setEventHandlers(RecvHandlerParam::EventHandler onConnect, RecvHandlerParam::EventHandler onMessage, RecvHandlerParam::EventHandler onDisconnect, void *funcParam) {
		this->onConnect = onConnect;
		this->onMessage = onMessage;
		this->onDisconnect = onDisconnect;
		this->funcParam = funcParam;
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
	void ClientSocketManager::disconnectSocket() {
		if (this->socketStatus == this->STATUS_DISCONNECTED) {
			return;
		} else if (this->socketStatus == this->STATUS_CONNECTED || this->socketStatus == this->STATUS_CONNECTING) { //connected, so shutdown immediately or terminate thread on next check
			Rain::shutdownSocketSend(this->socket);
			closesocket(this->socket);

			//this will exit the connect thread
			this->socketStatus = this->STATUS_DISCONNECTED;
		}
	}
	bool ClientSocketManager::setLogging(bool enable, void *logger) {
		bool ret = (this->logger != NULL);
		this->logger = reinterpret_cast<RainLogger *>(logger);
		return ret;
	}
	DWORD ClientSocketManager::attemptConnectThread(LPVOID param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		csm.msReconnectWait = 1;
		csm.portAddrs.clear();
		csm.portAddrs.resize(csm.highPort - csm.lowPort + 1, NULL);
		ResetEvent(csm.connectEvent);

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
					Rain::getTargetAddr(&csm.portAddrs[a - csm.lowPort], csm.ipAddress, Rain::tToStr(a));
				}

				//any socket that is connected will pass through this step
				if (!Rain::connectTarget(csm.socket, &csm.portAddrs[a - csm.lowPort])) {
					//set an event when connected for all listeners of the event
					SetEvent(csm.connectEvent);

					//attach the socket to a recvThread
					csm.rParam.bufLen = csm.recvBufLen;
					csm.rParam.funcParam = static_cast<void *>(&csm);
					csm.rParam.message = NULL;
					csm.rParam.onProcessMessage = csm.onProcessMessage;
					csm.rParam.onRecvInit = csm.onRecvInit;
					csm.rParam.onRecvExit = csm.onRecvExit;
					csm.rParam.socket = &csm.socket;
					Rain::createRecvThread(&csm.rParam);

					csm.socketStatus = csm.STATUS_CONNECTED;
					break;
				}
			}
		}

		return 0;
	}
	DWORD ClientSocketManager::attemptSendMessageThread(LPVOID param) {
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

		SetEvent(csm.messageDoneEvent);
		return 0;
	}
	int ClientSocketManager::onRecvInit(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		csm.rParam.message = new std::string();
		return csm.onConnect == NULL ? 0 : csm.onConnect(csm.funcParam);
	}
	int ClientSocketManager::onRecvExit(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		//retry to connect
		csm.socketStatus = csm.STATUS_CONNECTING;

		//create thread to attempt connect
		//thread exits when connect success
		Rain::simpleCreateThread(ClientSocketManager::attemptConnectThread, &csm);

		delete csm.rParam.message;
		return csm.onDisconnect == NULL ? 0 : csm.onDisconnect(csm.funcParam);
	}
	int ClientSocketManager::onProcessMessage(void *param) {
		Rain::ClientSocketManager &csm = *reinterpret_cast<Rain::ClientSocketManager *>(param);

		//if we are logging socket communications, do that here for incoming messages
		if (csm.logger != NULL)
			csm.logger->logString(csm.rParam.message);

		return csm.onMessage == NULL ? 0 : csm.onMessage(csm.funcParam);
	}
}