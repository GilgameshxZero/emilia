/*
Standard
*/

/*
Implements thread for listening for socket connections and spawning recvThreads.
*/

#pragma once

#include "UtilityError.h"
#include "NetworkBase.h"
#include "NetworkRecvThread.h"
#include "NetworkSocketManager.h"
#include "NetworkUtility.h"

#include <cstddef>
#include <mutex>

namespace Rain {
	class NetworkServerManager : public NetworkSocketManager {
		public:
		static const std::size_t RECV_BUF_LEN = 65536;

		NetworkServerManager();
		~NetworkServerManager();

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

		//returns current socket immediately
		SOCKET &getSocket();

		//change socket to listen on different ports, or start socket listen on some ports
		void setServerListen(DWORD lowPort, DWORD highPort);

		//set event handlers in addition to those of class
		//pass NULL to any parameter to remove the custom handler
		void setEventHandlers(NetworkRecvHandlerParam::EventHandler onConnect,
							  NetworkRecvHandlerParam::EventHandler onMessage,
							  NetworkRecvHandlerParam::EventHandler onDisconnect,
							  void *funcParam);

		//set send message attempt time max
		DWORD setSendAttemptTime(DWORD msMaxInterval);

		private:
		SOCKET socket;
		std::queue<std::string> messageQueue;
		bool blockSendRawMessage;
		int socketStatus;
		std::string ipAddress;
		DWORD lowPort, highPort;
		NetworkRecvHandlerParam::EventHandler onConnect, onMessage, onDisconnect;
		void *funcParam;
		DWORD msReconnectWaitMax, msSendWaitMax;

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
		Rain::NetworkRecvHandlerParam rParam;

		//disconnects socket immediately, regardless of state
		//sets state as -1
		void disconnectSocket();

		//thread function to attempt reconnects with time intervals
		static DWORD attemptConnectThread(LPVOID param);

		//thread function to attempt send messages on an interval
		static DWORD attemptSendMessageThread(LPVOID param);

		//event handlers for internal recvThread, before passing to delegates
		static int onRecvInit(void *param);
		static int onRecvExit(void *param);
		static int onProcessMessage(void *param);
	};
	//used to pass parameters from createListenThread to listenThread
	struct WSA2ListenThreadParam {
		//same variables as createListenThread - check there for what these do
		SOCKET *lSocket;
		void *recvFuncParam;
		std::size_t recvBufferLength;
		NetworkRecvHandlerParam::EventHandler onProcessMessage;
		NetworkRecvHandlerParam::EventHandler onRecvInit;
		NetworkRecvHandlerParam::EventHandler onRecvExit;
	};

	//used to pass parameters from listenThread spawned recvThreads to recv handlers specified in createListenThread
	struct WSA2ListenThreadRecvFuncDelegateParam {
		//socket used by this recvThread
		SOCKET *cSocket;

		//message used by the recvThread
		std::string message;

		//parameter to be dynamically set and used by the delegate recv handlers
		void *delegateParam;

		//parameter passed from createListenThread
		void *callerParam;
	};

	//used to pass parameters from listenThread to spawned recvThreads
	struct WSA2ListenThreadRecvFuncParam {
		//used by recvThread handlers to pass to their delegate handlers
		WSA2ListenThreadRecvFuncDelegateParam ltrfdParam;

		//keep track of NetworkRecvHandlerParam, which needs to be freed on recvThread exit
		NetworkRecvHandlerParam *pRFParam;

		//position in a linked list of ltrfParam, which should be modified when recvThread inits or exits
		WSA2ListenThreadRecvFuncParam *prevLTRFP, *nextLTRFP;

		//mutex to lock when changing linked list
		//todo: don't lock whole linked list - just the positions around where we are modifying
		std::mutex *llMutex;

		//access WSA2ListenThreadParam to access the delegate handlers
		WSA2ListenThreadParam *pLTParam;

		//handle to the current recvThread for use when waiting on end
		HANDLE hRecvThread;
	};

	//creates thread which accepts on a specified listening socket, until the socket is closed from outside the thread
	//accepted connections will spawn a recvThread for each connection with specified parameters
	//any parameters that the onProc functions will need to use will need to be created in onRecvInit - there will be space to store the parameters to be passed on the functions through the use of a void * in ListenThreadRecvParam
	//when listening socket is closed, thread will close all connection sockets and wait until all spawned recvThreads exit
	HANDLE createListenThread(SOCKET &lSocket,
							  void *recvFuncParam, //parameter to be passed to handler delegates
							  std::size_t recvBufferLength,
							  Rain::NetworkRecvHandlerParam::EventHandler onProcessMessage,
							  Rain::NetworkRecvHandlerParam::EventHandler onRecvInit,
							  Rain::NetworkRecvHandlerParam::EventHandler onRecvExit);
	
	//don't use externally
	//implements the listen thread specified above
	DWORD WINAPI listenThread(LPVOID lpParameter);

	//handlers used by the listenThread, which eventually call the corresponding handlers passed to createListenThread
	int onListenThreadRecvInit(void *funcParam);
	int onListenThreadRecvExit(void *funcParam);
	int onListenThreadRecvProcessMessage(void *funcParam);
}