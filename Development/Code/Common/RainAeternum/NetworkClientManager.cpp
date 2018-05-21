#include "NetworkClientManager.h"

namespace Rain {
	NetworkClientManager::NetworkClientManager() {
		this->socket = NULL;
		this->blockSendRawMessage = false;
		this->socketStatus = -1;
		this->ipAddress = "";
		this->lowPort = this->highPort = 0;
		this->onConnect = this->onMessage = this->onDisconnect = NULL;
		this->msReconnectAttempt = 5000;
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
		}
	}
	void NetworkClientManager::setEventHandlers(EventHandlerFunc onConnect, EventHandlerFunc onMessage, EventHandlerFunc onDisconnect) {
		this->onConnect = onConnect;
		this->onMessage = onMessage;
		this->onDisconnect = onDisconnect;
	}
	DWORD NetworkClientManager::setReconnectAttemptTime(DWORD msMaxInterval) {
		DWORD origValue = this->msReconnectAttempt;
		this->msReconnectAttempt = msMaxInterval;
		return origValue;
	}
}