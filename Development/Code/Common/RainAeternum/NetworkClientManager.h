/*
Standard
*/

/*
Implements class ClientSocketManager, which maintains a socket connection to an address & port range, allowing for timeouts and error management along the way. If a socket connection is terminated, the class will attempt to reconnect until stopped or successful.
*/

#pragma once

#include "NetworkBase.h"
#include "NetworkRecvThread.h"
#include "NetworkSocketManager.h"
#include "NetworkUtility.h"

#include "UtilityLogging.h"

#include <string>
#include <queue>
#include <vector>

namespace Rain {
	class ClientSocketManager : public SocketManager {
		public:
		static const int STATUS_DISCONNECTED = -1,
			STATUS_CONNECTED = 0,
			STATUS_CONNECTING = 1;

		ClientSocketManager();
		~ClientSocketManager();

		//sends raw message over socket
		//non-blocking by default; if socket is unusable, will delay send until socket is usable again
		//messages are sent without buffering
		void sendRawMessage(std::string request);

		//if blocking, passing by pointer would be faster, avoiding copying of string and sending directly
		void sendRawMessage(std::string *request);

		void clearMessageQueue();

		//wait until timeout elapses or queued messages are sent
		//0 for infinite
		void blockForMessageQueue(DWORD msTimeout = 5000);

		//set sendRawMessage as blocking or non-blocking
		bool setSendRawMessageBlocking(bool blocking);
		
		//-1: socket is disconnected; 0: socket is connected; 1: socket is trying to connect but currently unavailable
		int getSocketStatus();

		//blocks until either timeout elapses or socket connects
		//0 for infinite
		void blockForConnect(DWORD msTimeout = 10000);

		//returns current socket immediately
		SOCKET &getSocket();

		//if called with non-empty ipAddress, setClientTarget will tell class to begin trying to connect to the target
		//an empty ipAddress terminates any existing connection and will not reconnect
		//if called, will terminate current connection
		void setClientTarget(std::string ipAddress, DWORD lowPort, DWORD highPort);

		//set event handlers in addition to those of class
		//pass NULL to any parameter to remove the custom handler
		void setEventHandlers(RecvHandlerParam::EventHandler onConnect, 
							  RecvHandlerParam::EventHandler onMessage, 
							  RecvHandlerParam::EventHandler onDisconnect,
							  void *funcParam);

		//set reconnect attempt time max from default 3000
		//will attempt to reconnect more often at the beginning, then slow down exponentially
		DWORD setReconnectAttemptTime(DWORD msMaxInterval);

		//set send message attempt time max
		DWORD setSendAttemptTime(DWORD msMaxInterval);

		//sets buffer length of recv thread
		std::size_t setRecvBufLen(std::size_t newLen);

		//inheirited from SocketManager; sets logging on and off for communications on this socket; pass NULL to disable
		bool setLogging(void *logger);

		private:
		SOCKET socket;
		std::queue<std::string> messageQueue;
		bool blockSendRawMessage;
		int socketStatus;
		std::string ipAddress;
		DWORD lowPort, highPort;
		RecvHandlerParam::EventHandler onConnect, onMessage, onDisconnect;
		void *funcParam;
		DWORD msReconnectWaitMax, msSendWaitMax;
		std::size_t recvBufLen;
		RainLogger *logger;

		//current wait between connect attempts
		DWORD msReconnectWait;

		//current wait between send message attempts
		DWORD msSendMessageWait;

		//addresses of all the ports
		std::vector<struct addrinfo *> portAddrs;

		//will be triggerred while socket is connected
		HANDLE connectEvent;

		//triggerred when message queue is empty
		HANDLE messageDoneEvent;

		//recvThread parameter associated with the current recvThread
		Rain::RecvHandlerParam rParam;

		//disconnects socket immediately, regardless of state
		//sets state as -1
		void disconnectSocket();

		//frees all the memory allocated for addresses
		void freePortAddrs();

		//thread function to attempt reconnects with time intervals
		static DWORD WINAPI attemptConnectThread(LPVOID param);

		//thread function to attempt send messages on an interval
		static DWORD WINAPI attemptSendMessageThread(LPVOID param);

		//event handlers for internal recvThread, before passing to delegates
		static int onRecvInit(void *param);
		static int onRecvExit(void *param);
		static int onProcessMessage(void *param);
	};
}