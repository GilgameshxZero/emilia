/*
Standard
*/

/*
Implements class NetworkClientManager, which maintains a socket connection to an address & port range, allowing for timeouts and error management along the way. If a socket connection is terminated, the class will attempt to reconnect until stopped or successful.
*/

#pragma once

#include "RainWSA2Include.h"

#include <string>
#include <queue>

namespace Rain {
	class NetworkClientManager {
		public:
		//event handler format
		typedef int(*EventHandlerFunc) (void *);

		NetworkClientManager();

		//sends raw message over socket
		//non-blocking by default; if socket is unusable, will delay send until socket is usable again
		//messages are sent without buffering
		DWORD sendRawMessage(std::string request);

		//if blocking, passing by pointer would be faster, avoiding copying of string and sending directly
		DWORD sendRawMessage(std::string *request);

		//wait until timeout elapses or queued messages are sent
		void blockForMessageQueue(DWORD msTimeout = 5000);

		//set sendRawMessage as blocking or non-blocking
		bool setSendRawMessageBlocking(bool blocking);
		
		//-1: socket is disconnected; 0: socket is connected; 1: socket is trying to connect but currently unavailable
		int getSocketStatus();

		//blocks until either timeout elapses or socket connects
		void blockForConnect(DWORD msTimeout = 10000);

		//returns a usable socket if possible, or NULL otherwise
		SOCKET &getSocket();

		//if called with non-empty ipAddress, setClientTarget will tell class to begin trying to connect to the target
		//an empty ipAddress terminates any existing connection and will not reconnect
		//if called, will terminate current connection
		void setClientTarget(std::string ipAddress, DWORD lowPort, DWORD highPort);

		//set event handlers in addition to those of class
		//pass NULL to any parameter to remove the custom handler
		void setEventHandlers(EventHandlerFunc onConnect, EventHandlerFunc onMessage, EventHandlerFunc onDisconnect);

		//set reconnect attempt times from default 5000
		//will attempt to reconnect more often at the beginning, then slow down exponentially
		DWORD setReconnectAttemptTime(DWORD msMaxInterval);

		private:
		SOCKET socket;
		std::queue<std::string> messageQueue;
		bool blockSendRawMessage;
		int socketStatus;
		std::string ipAddress;
		DWORD lowPort, highPort;
		EventHandlerFunc onConnect, onMessage, onDisconnect;
		DWORD msReconnectAttempt;

		//disconnects socket immediately
		void disconnectSocket();
	};
}