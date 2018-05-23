#include "NetworkClientManager.h"

namespace Rain {
	NetworkClientManager::NetworkClientManager() {
		this->socket = NULL;
		this->blockSendRawMessage = false;
		this->socketStatus = -1;
		this->ipAddress = "";
		this->lowPort = this->highPort = 0;
		this->onConnect = this->onMessage = this->onDisconnect = NULL;
		this->funcParam = NULL;
		this->msReconnectWaitMax = this->msSendWaitMax = 3000;

		this->connectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		this->messageDoneEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

		Rain::initWinsock22();
	}
	NetworkClientManager::~NetworkClientManager() {
		CloseHandle(this->connectEvent);

		//this automatically closes the send thread
		CloseHandle(this->messageDoneEvent);

		//shutsdown connect threads, if any
		this->socketStatus = this->STATUS_DISCONNECTED;

		Rain::shutdownSocketSend(this->socket);
		closesocket(this->socket);

	}
	void NetworkClientManager::sendRawMessage(std::string request) {
		this->sendRawMessage(&request);
	}
	void NetworkClientManager::sendRawMessage(std::string *request) {
		//uses copy constructor
		this->messageQueue.push(*request);

		if (WaitForSingleObject(this->messageDoneEvent, 1) == WAIT_OBJECT_0) { //means that the object was set/the thread is not active currently
			//reset event outside of thread start so multiple calls to sendRawMessage won't create race conditions
			ResetEvent(this->messageDoneEvent);

			//start thread to send messages
			Rain::simpleCreateThread(NetworkClientManager::attemptSendMessageThread, this);
		}
	}
	void NetworkClientManager::clearMessageQueue() {
		this->messageQueue = std::queue<std::string>();
	}
	void NetworkClientManager::blockForMessageQueue(DWORD msTimeout) {
		WaitForSingleObject(this->messageDoneEvent, msTimeout == 0 ? INFINITE : msTimeout);
	}
	bool NetworkClientManager::setSendRawMessageBlocking(bool blocking) {
		bool origValue = this->blockSendRawMessage;
		this->blockSendRawMessage = blocking;
		return origValue;
	}
	int NetworkClientManager::getSocketStatus() {
		return this->socketStatus;
	}
	void NetworkClientManager::blockForConnect(DWORD msTimeout) {
		WaitForSingleObject(this->connectEvent, msTimeout == 0 ? INFINITE : msTimeout);
	}
	SOCKET &NetworkClientManager::getSocket() {
		return this->socket;
	}
	void NetworkClientManager::setClientTarget(std::string ipAddress, DWORD lowPort, DWORD highPort) {
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
			Rain::simpleCreateThread(NetworkClientManager::attemptConnectThread, this);
		}
	}
	void NetworkClientManager::setEventHandlers(NetworkRecvHandlerParam::EventHandler onConnect, NetworkRecvHandlerParam::EventHandler onMessage, NetworkRecvHandlerParam::EventHandler onDisconnect, void *funcParam) {
		this->onConnect = onConnect;
		this->onMessage = onMessage;
		this->onDisconnect = onDisconnect;
		this->funcParam = funcParam;
	}
	DWORD NetworkClientManager::setReconnectAttemptTime(DWORD msMaxInterval) {
		DWORD origValue = this->msReconnectWaitMax;
		this->msReconnectWaitMax = msMaxInterval;
		return origValue;
	}
	DWORD NetworkClientManager::setSendAttemptTime(DWORD msMaxInterval) {
		DWORD origValue = this->msSendWaitMax;
		this->msSendWaitMax = msMaxInterval;
		return origValue;
	}
	void NetworkClientManager::disconnectSocket() {
		if (this->socketStatus == this->STATUS_DISCONNECTED) {
			return;
		} else if (this->socketStatus == this->STATUS_CONNECTED || this->socketStatus == this->STATUS_CONNECTING) { //connected, so shutdown immediately or terminate thread on next check
			Rain::shutdownSocketSend(this->socket);
			closesocket(this->socket);

			//this will exit the connect thread
			this->socketStatus = this->STATUS_DISCONNECTED;
		}
	}
	DWORD NetworkClientManager::attemptConnectThread(LPVOID param) {
		Rain::NetworkClientManager &ncm = *reinterpret_cast<Rain::NetworkClientManager *>(param);

		ncm.msReconnectWait = 1;
		ncm.portAddrs.clear();
		ncm.portAddrs.resize(ncm.highPort - ncm.lowPort + 1, NULL);
		ResetEvent(ncm.connectEvent);

		Rain::createSocket(ncm.socket);
		
		while (ncm.socketStatus == ncm.STATUS_CONNECTING) {
			Sleep(ncm.msReconnectWait);
			if (ncm.msReconnectWait < ncm.msReconnectWaitMax)
				ncm.msReconnectWait = min(ncm.msReconnectWait * 2, ncm.msReconnectWaitMax);

			//try to reconnect; assume ipAddress and port ranges are valid
			//if address has not yet been retreived, try to get it here
			for (DWORD a = ncm.lowPort; a <= ncm.highPort; a++) {
				if (ncm.socketStatus != ncm.STATUS_CONNECTING) //make sure status hasn't changed externally
					break;

				if (ncm.portAddrs[a - ncm.lowPort] == NULL) { //address not yet found, get it now
					Rain::getTargetAddr(&ncm.portAddrs[a - ncm.lowPort], ncm.ipAddress, Rain::tToStr(a));
				}

				//any socket that is connected will pass through this step
				if (!Rain::connectTarget(ncm.socket, &ncm.portAddrs[a - ncm.lowPort])) {
					//set an event when connected for all listeners of the event
					SetEvent(ncm.connectEvent);

					//attach the socket to a recvThread
					ncm.rParam.bufLen = ncm.RECV_BUF_LEN;
					ncm.rParam.funcParam = static_cast<void *>(&ncm);
					ncm.rParam.message = NULL;
					ncm.rParam.onProcessMessage = ncm.onProcessMessage;
					ncm.rParam.onRecvInit = ncm.onRecvInit;
					ncm.rParam.onRecvExit = ncm.onRecvExit;
					ncm.rParam.socket = &ncm.socket;
					Rain::createRecvThread(&ncm.rParam);

					ncm.socketStatus = ncm.STATUS_CONNECTED;
					break;
				}
			}
		}

		return 0;
	}
	DWORD NetworkClientManager::attemptSendMessageThread(LPVOID param) {
		Rain::NetworkClientManager &ncm = *reinterpret_cast<Rain::NetworkClientManager *>(param);

		ncm.msSendMessageWait = 1;

		//attempt to send messages until ncm.messageQueue is empty, at which point set the messageDoneEvent
		while (!ncm.messageQueue.empty()) {
			ncm.blockForConnect(ncm.msSendMessageWait);

			//deal with differently depending on whether we connected on that block or not
			if (ncm.socketStatus != ncm.STATUS_CONNECTED &&
				ncm.msSendMessageWait < ncm.msSendWaitMax)
				ncm.msSendMessageWait = min(ncm.msSendMessageWait * 2, ncm.msSendWaitMax);
			else if (ncm.socketStatus == ncm.STATUS_CONNECTED) {
				Rain::sendRawMessage(ncm.socket, &ncm.messageQueue.front());
				ncm.messageQueue.pop();
			}
		}

		SetEvent(ncm.messageDoneEvent);
		return 0;
	}
	int NetworkClientManager::onRecvInit(void *param) {
		Rain::NetworkClientManager &ncm = *reinterpret_cast<Rain::NetworkClientManager *>(param);

		ncm.rParam.message = new std::string();
		return ncm.onConnect == NULL ? 0 : ncm.onConnect(ncm.funcParam);
	}
	int NetworkClientManager::onRecvExit(void *param) {
		Rain::NetworkClientManager &ncm = *reinterpret_cast<Rain::NetworkClientManager *>(param);

		//retry to connect
		ncm.socketStatus = ncm.STATUS_CONNECTING;

		//create thread to attempt connect
		//thread exits when connect success
		Rain::simpleCreateThread(NetworkClientManager::attemptConnectThread, &ncm);

		delete ncm.rParam.message;
		return ncm.onDisconnect == NULL ? 0 : ncm.onDisconnect(ncm.funcParam);
	}
	int NetworkClientManager::onProcessMessage(void *param) {
		Rain::NetworkClientManager &ncm = *reinterpret_cast<Rain::NetworkClientManager *>(param);
		return ncm.onMessage == NULL ? 0 : ncm.onMessage(ncm.funcParam);
	}
}