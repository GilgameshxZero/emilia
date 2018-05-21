#include "NetworkClientManager.h"

namespace Rain {
	const int NetworkClientManager::STATUS_DISCONNECTED = -1,
		NetworkClientManager::STATUS_CONNECTED = 0,
		NetworkClientManager::STATUS_CONNECTING = 1;
	NetworkClientManager::NetworkClientManager() {
		this->socket = NULL;
		this->blockSendRawMessage = false;
		this->socketStatus = -1;
		this->ipAddress = "";
		this->lowPort = this->highPort = 0;
		this->onConnect = this->onMessage = this->onDisconnect = NULL;
		this->msReconnectWaitMax = 3000;

		//if winsock is not initialized, do that
		//discard WSADATA
		WSADATA wsaData;
		Rain::initWinsock(wsaData);
	}
	DWORD NetworkClientManager::sendRawMessage(std::string request) {
		this->sendRawMessage(&request);
	}
	DWORD NetworkClientManager::sendRawMessage(std::string *request) {

	}
	void NetworkClientManager::blockForMessageQueue(DWORD msTimeout) {

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

	}
	SOCKET &NetworkClientManager::getSocket() {

	}
	void NetworkClientManager::setClientTarget(std::string ipAddress, DWORD lowPort, DWORD highPort) {
		this->ipAddress = ipAddress;
		if (this->ipAddress.length() == 0) {
			this->lowPort = this->highPort = 0;
		} else {
			this->lowPort = lowPort;
			this->highPort = highPort;

			this->disconnectSocket();
			this->socketStatus = this->STATUS_CONNECTING;
			this->msReconnectWait = 1;

			//create thread to attempt connect
			Rain::simpleCreateThread(this->attemptConnectThread, this);
		}
	}
	void NetworkClientManager::setEventHandlers(NetworkRecvHandlerParam::EventHandler onConnect, NetworkRecvHandlerParam::EventHandler onMessage, NetworkRecvHandlerParam::EventHandler onDisconnect) {
		this->onConnect = onConnect;
		this->onMessage = onMessage;
		this->onDisconnect = onDisconnect;
	}
	DWORD NetworkClientManager::setReconnectAttemptTime(DWORD msMaxInterval) {
		DWORD origValue = this->msReconnectWaitMax;
		this->msReconnectWaitMax = msMaxInterval;
		return origValue;
	}
	void NetworkClientManager::disconnectSocket() {
		if (this->socketStatus == this->STATUS_DISCONNECTED) {
			return;
		} else if (this->socketStatus == this->STATUS_CONNECTED || this->socketStatus == this->STATUS_CONNECTING) { //connected, so shutdown immediately or terminate thread on next check
			Rain::shutdownSocketSend(this->socket);
			closesocket(this->socket);
			this->socketStatus = this->STATUS_DISCONNECTED;
		}
	}
	void NetworkClientManager::attemptConnectThread(void *param) {
		Rain::NetworkClientManager &ncm = *reinterpret_cast<Rain::NetworkClientManager *>(param);
		
		while (ncm.socketStatus == ncm.STATUS_CONNECTING) {
			//try to reconnect; assume ipAddress and port ranges are valid


			Sleep(ncm.msReconnectWait);
			if (ncm.msReconnectWait < ncm.msReconnectWaitMax)
				ncm.msReconnectWait = min(ncm.msReconnectWait * 2, ncm.msReconnectWaitMax);
		}
	}
}