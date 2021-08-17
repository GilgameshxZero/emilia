#include "network-headed-managers.h"

namespace Rain {
	void HeadedClientSocketManager::setEventHandlers(RecvHandlerParam::EventHandler onConnect, RecvHandlerParam::EventHandler onMessage, RecvHandlerParam::EventHandler onDisconnect, void *funcParam) {
		//handlers should go to the headed handlers, then go to these delegate handlers
		this->onHeadedConnectDelegate = onConnect;
		this->onHeadedMessageDelegate = onMessage;
		this->onHeadedDisconnectDelegate = onDisconnect;
		this->headedDelegateParam = funcParam;

		ClientSocketManager::onConnectDelegate = onHeadedConnect;
		ClientSocketManager::onMessageDelegate = onHeadedMessage;
		ClientSocketManager::onDisconnectDelegate = onHeadedDisconnect;
		ClientSocketManager::csmdhParam.delegateParam = this;
	}
	int HeadedClientSocketManager::onHeadedConnect(void *param) {
		ClientSocketManager::DelegateHandlerParam &csmdhp = *reinterpret_cast<ClientSocketManager::DelegateHandlerParam *>(param);
		HeadedClientSocketManager &hcsm = *reinterpret_cast<HeadedClientSocketManager *>(csmdhp.delegateParam);
		hcsm.messageLength = 0;
		hcsm.csmdhp.csm = csmdhp.csm;
		hcsm.csmdhp.delegateParam = hcsm.headedDelegateParam;
		hcsm.csmdhp.message = &hcsm.accMess;
		return hcsm.onHeadedConnectDelegate == NULL ? 0 : hcsm.onHeadedConnectDelegate(reinterpret_cast<void *>(&hcsm.csmdhp));
	}
	int HeadedClientSocketManager::onHeadedMessage(void *param) {
		ClientSocketManager::DelegateHandlerParam &csmdhp = *reinterpret_cast<ClientSocketManager::DelegateHandlerParam *>(param);
		HeadedClientSocketManager &hcsm = *reinterpret_cast<HeadedClientSocketManager *>(csmdhp.delegateParam);
		hcsm.accMess += *csmdhp.message;
		int ret = 0;
		while (isMessageComplete(hcsm.messageLength, hcsm.accMess)) {
			std::string fragment = hcsm.accMess.substr(hcsm.messageLength, hcsm.accMess.length());
			hcsm.accMess = hcsm.accMess.substr(0, hcsm.messageLength);
			int delRet = hcsm.onHeadedMessageDelegate == NULL ? 0 : hcsm.onHeadedMessageDelegate(reinterpret_cast<void *>(&hcsm.csmdhp));
			ret = ret == 0 ? delRet : ret;
			hcsm.accMess = fragment;
			hcsm.messageLength = 0;
		}
		return ret;
	}
	int HeadedClientSocketManager::onHeadedDisconnect(void *param) {
		ClientSocketManager::DelegateHandlerParam &csmdhp = *reinterpret_cast<ClientSocketManager::DelegateHandlerParam *>(param);
		HeadedClientSocketManager &hcsm = *reinterpret_cast<HeadedClientSocketManager *>(csmdhp.delegateParam);
		return hcsm.onHeadedDisconnectDelegate == NULL ? 0 : hcsm.onHeadedDisconnectDelegate(reinterpret_cast<void *>(&hcsm.csmdhp));
	}

	void HeadedServerManager::setEventHandlers(RecvHandlerParam::EventHandler onConnect, RecvHandlerParam::EventHandler onMessage, RecvHandlerParam::EventHandler onDisconnect, void *funcParam) {
		//handlers should go to the headed handlers, then go to these delegate handlers
		this->onHeadedConnectDelegate = onConnect;
		this->onHeadedMessageDelegate = onMessage;
		this->onHeadedDisconnectDelegate = onDisconnect;
		this->headedDelegateParam = funcParam;

		ServerManager::onConnectDelegate = onHeadedConnect;
		ServerManager::onMessageDelegate = onHeadedMessage;
		ServerManager::onDisconnectDelegate = onHeadedDisconnect;
		ServerManager::funcParam = this;
	}
	int HeadedServerManager::onHeadedConnect(void *param) {
		ServerSocketManager::DelegateHandlerParam &ssmdhp = *reinterpret_cast<ServerSocketManager::DelegateHandlerParam *>(param);
		HeadedServerManager &hsm = *reinterpret_cast<HeadedServerManager *>(ssmdhp.callerParam);

		ssmdhp.delegateParam = new DelegateHandlerParam();
		DelegateHandlerParam &dhp = *reinterpret_cast<DelegateHandlerParam *>(ssmdhp.delegateParam);
		dhp.ssmdhp.callerParam = hsm.headedDelegateParam;
		dhp.ssmdhp.cSocket = ssmdhp.cSocket;
		dhp.ssmdhp.delegateParam = NULL;
		dhp.ssmdhp.message = &dhp.accMess;
		dhp.ssmdhp.ssm = ssmdhp.ssm;

		dhp.messageLength = 0;

		return hsm.onHeadedConnectDelegate == NULL ? 0 : hsm.onHeadedConnectDelegate(reinterpret_cast<void *>(&dhp.ssmdhp));
	}
	int HeadedServerManager::onHeadedMessage(void *param) {
		ServerSocketManager::DelegateHandlerParam &ssmdhp = *reinterpret_cast<ServerSocketManager::DelegateHandlerParam *>(param);
		HeadedServerManager &hsm = *reinterpret_cast<HeadedServerManager *>(ssmdhp.callerParam);
		DelegateHandlerParam &dhp = *reinterpret_cast<DelegateHandlerParam *>(ssmdhp.delegateParam);
		dhp.accMess += *ssmdhp.message;
		int ret = 0;
		while (isMessageComplete(dhp.messageLength, dhp.accMess)) {
			std::string fragment = dhp.accMess.substr(dhp.messageLength, dhp.accMess.length());
			dhp.accMess = dhp.accMess.substr(0, dhp.messageLength);
			int delRet = hsm.onHeadedMessageDelegate == NULL ? 0 : hsm.onHeadedMessageDelegate(reinterpret_cast<void *>(&dhp.ssmdhp));
			ret = ret == 0 ? delRet : ret;
			dhp.accMess = fragment;
			dhp.messageLength = 0;
		}
		return ret;
	}
	int HeadedServerManager::onHeadedDisconnect(void *param) {
		ServerSocketManager::DelegateHandlerParam &ssmdhp = *reinterpret_cast<ServerSocketManager::DelegateHandlerParam *>(param);
		HeadedServerManager &hsm = *reinterpret_cast<HeadedServerManager *>(ssmdhp.callerParam);
		DelegateHandlerParam &dhp = *reinterpret_cast<DelegateHandlerParam *>(ssmdhp.delegateParam);
		int ret = hsm.onHeadedDisconnectDelegate == NULL ? 0 : hsm.onHeadedDisconnectDelegate(reinterpret_cast<void *>(&dhp.ssmdhp));
		delete &dhp;
		return ret;
	}

	bool isMessageComplete(std::size_t &messageLength, std::string &accMess) {
		if (messageLength == 0) {
			if (accMess.length() < 2) {
				return false;
			} else {
				messageLength =
					static_cast<unsigned char>(accMess[0]) << 8 |
					static_cast<unsigned char>(accMess[1]);
				if (messageLength == 0) {
					//message length is 4-byte int, for a total 6-byte header
					if (accMess.length() < 6) {
						return false;
					} else {
						messageLength =
							static_cast<unsigned char>(accMess[2]) << 24 |
							static_cast<unsigned char>(accMess[3]) << 16 |
							static_cast<unsigned char>(accMess[4]) << 8 |
							static_cast<unsigned char>(accMess[5]);
						accMess = accMess.substr(6, accMess.length());
					}
				} else {
					//message length is 2-byte int for a 2-byte header
					messageLength =
						static_cast<unsigned char>(accMess[0]) << 8 |
						static_cast<unsigned char>(accMess[1]);
					accMess = accMess.substr(2, accMess.length());
				}
			}
		}

		if (messageLength != 0) {
			return accMess.length() >= messageLength;
		}

		return false;
	}
}