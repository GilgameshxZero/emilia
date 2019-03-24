/*
Standard
*/

/*
Extends ClientSocketManager and ServerManager with the `headed` protocol, which prepends each message with a length header, and only sends full messages to the delegate handler. The header is a big endian 2-byte integer for messages under 65536 length, and for larger messages, would have the first 2 bytes be 0, and the following 4 bytes be a big endian 4-byte integer for the length of the message (6 bytes total).
*/

#pragma once

#include "network-client-manager.h"
#include "network-server-manager.h"

namespace Rain {
	class HeadedClientSocketManager : public ClientSocketManager {
	public:
		//override
		void setEventHandlers(RecvHandlerParam::EventHandler onConnect,
			RecvHandlerParam::EventHandler onMessage,
			RecvHandlerParam::EventHandler onDisconnect,
			void *funcParam);

	private:
		std::size_t messageLength;
		std::string accMess;
		ClientSocketManager::DelegateHandlerParam csmdhp;

		RecvHandlerParam::EventHandler onHeadedConnectDelegate, onHeadedMessageDelegate, onHeadedDisconnectDelegate;
		void *headedDelegateParam;

		static int onHeadedConnect(void *param);
		static int onHeadedMessage(void *param);
		static int onHeadedDisconnect(void *param);
	};

	class HeadedServerManager : public ServerManager {
	public:
		struct DelegateHandlerParam {
			std::size_t messageLength = 0;
			std::string accMess;

			//param passed to true delegates
			ServerSocketManager::DelegateHandlerParam ssmdhp;
		};

		//override
		void setEventHandlers(RecvHandlerParam::EventHandler onConnect,
			RecvHandlerParam::EventHandler onMessage,
			RecvHandlerParam::EventHandler onDisconnect,
			void *funcParam);

	private:
		RecvHandlerParam::EventHandler onHeadedConnectDelegate, onHeadedMessageDelegate, onHeadedDisconnectDelegate;
		void *headedDelegateParam;

		static int onHeadedConnect(void *param);
		static int onHeadedMessage(void *param);
		static int onHeadedDisconnect(void *param);
	};

	bool isMessageComplete(std::size_t &messageLength, std::string &accMess);
}