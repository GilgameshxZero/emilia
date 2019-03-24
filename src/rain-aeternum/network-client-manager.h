/*
Standard
*/

/*
Implements class ClientSocketManager, which maintains a socket connection to an address & port range, allowing for timeouts and error management along the way. If a socket connection is terminated, the class will attempt to reconnect until stopped or successful.
*/

#pragma once

#include "network-base.h"
#include "network-recv-thread.h"
#include "network-socket-manager.h"
#include "network-utility.h"
#include "utility-logging.h"

#include <algorithm>
#include <string>
#include <queue>
#include <vector>

namespace Rain {
	class ClientSocketManager : public SocketManager {
	public:
		struct DelegateHandlerParam {
			//the current csm
			ClientSocketManager *csm;

			//message received
			std::string *message;

			//additional parameters set by setHandlers
			void *delegateParam;
		};

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
		//INFINITE for infinite
		void blockForConnect(DWORD msTimeout = 10000);

		//returns value of connected port, or -1 if not connected
		DWORD getConnectedPort();

		//returns domain name or IP, whichever was set in setClientTarget, or "" if not connected
		std::string getTargetIP();

		//returns current socket immediately
		SOCKET &getSocket();

		//if called with non-empty ipAddress, setClientTarget will tell class to begin trying to connect to the target
		//an empty ipAddress terminates any existing connection and will not reconnect
		//if called, will terminate current connection
		void setClientTarget(std::string ipAddress, DWORD lowPort, DWORD highPort);

		//if disconnected, start trying to connect again
		void retryConnect();

		//whether to retry connection when disconnect; returns previous value
		bool setRetryOnDisconnect(bool value);

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

	protected:
		//have the message handlers be protected so that derived classes can access them
		RecvHandlerParam::EventHandler onConnectDelegate, onMessageDelegate, onDisconnectDelegate;

		//parameter passed to delegate handlers
		DelegateHandlerParam csmdhParam;

	private:
		SOCKET socket;
		std::queue<std::string> messageQueue;
		bool blockSendRawMessage;
		DWORD connectedPort;
		int socketStatus;
		std::string ipAddress;
		DWORD lowPort, highPort;
		DWORD msReconnectWaitMax, msSendWaitMax;
		std::size_t recvBufLen;
		LogStream *logger;

		//set to true when destructor called; terminate send thread here
		bool destructing;

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

		//triggered when message queue not empty again
		HANDLE messageToSendEvent;

		//recvThread parameter associated with the current recvThread
		Rain::RecvHandlerParam rParam;

		//handle to the connect thread and send message thread
		//close when threads end by the thread
		HANDLE hConnectThread, hSendThread;

		//event which will be set when there have been as many calls to onDisconnect as there have been to onConnect
		HANDLE recvExitComplete;

		//lock this when modifying message queue
		std::mutex queueMutex;

		//if set to true, CSM will attempt to reconnect on disconnect ONCE
		bool retryOnDisconnect;

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
		static int onConnect(void *param);
		static int onMessage(void *param);
		static int onDisconnect(void *param);
	};
}