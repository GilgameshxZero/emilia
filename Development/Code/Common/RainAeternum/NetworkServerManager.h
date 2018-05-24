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

	//used to pass parameters from createListenThread to listenThread
	struct WSA2ListenThreadParam {
		//same variables as createListenThread - check there for what these do
		SOCKET *lSocket;
		void *recvFuncParam;
		std::size_t recvBufferLength;
		RecvHandlerParam::EventHandler onProcessMessage;
		RecvHandlerParam::EventHandler onRecvInit;
		RecvHandlerParam::EventHandler onRecvExit;
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

		//keep track of RecvHandlerParam, which needs to be freed on recvThread exit
		RecvHandlerParam *pRFParam;

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
							  Rain::RecvHandlerParam::EventHandler onProcessMessage,
							  Rain::RecvHandlerParam::EventHandler onRecvInit,
							  Rain::RecvHandlerParam::EventHandler onRecvExit);

	//don't use externally
	//implements the listen thread specified above
	DWORD WINAPI listenThread(LPVOID lpParameter);

	//handlers used by the listenThread, which eventually call the corresponding handlers passed to createListenThread
	int onListenThreadRecvInit(void *funcParam);
	int onListenThreadRecvExit(void *funcParam);
	int onListenThreadRecvProcessMessage(void *funcParam);

	//manages a single server/client connection, from the server-end
	class ServerSocketManager : public SocketManager {
		public:
		ServerSocketManager();
		~ServerSocketManager();

		//sends raw message over socket, without queueing or blocking
		void sendRawMessage(std::string request);
		void sendRawMessage(std::string *request);

		//returns current socket immediately
		SOCKET &getSocket();

		//set event handlers in addition to those of class
		//pass NULL to any parameter to remove the custom handler
		void setEventHandlers(RecvHandlerParam::EventHandler onConnect,
							  RecvHandlerParam::EventHandler onMessage,
							  RecvHandlerParam::EventHandler onDisconnect,
							  void *funcParam);

		private:
		SOCKET socket;
		RecvHandlerParam::EventHandler onConnect, onMessage, onDisconnect;
		void *funcParam;
		RainLogger *logger;

		//recvThread parameter associated with the current recvThread
		Rain::RecvHandlerParam rParam;

		//inheirited from SocketManager; sets logging on and off for communications on this socket
		bool setLogging(bool enable, void *logger);

		//event handlers for internal recvThread, before passing to delegates
		static int onRecvInit(void *param);
		static int onRecvExit(void *param);
		static int onProcessMessage(void *param);
	};

	//spawns a ServerSocketManager for each new connection
	class ServerManager {
		public:
		ServerManager();
		~ServerManager();

		//returns current socket immediately
		SOCKET &getSocket();

		//change socket to listen on different ports, or start socket listen on some ports
		void setServerListen(DWORD lowPort, DWORD highPort);

		//set event handlers in addition to those of class
		//pass NULL to any parameter to remove the custom handler
		void setEventHandlers(RecvHandlerParam::EventHandler onConnect,
							  RecvHandlerParam::EventHandler onMessage,
							  RecvHandlerParam::EventHandler onDisconnect,
							  void *funcParam);

		//sets buffer length of recv thread
		std::size_t setRecvBufLen(std::size_t newLen);

		private:
		SOCKET socket;
		DWORD lowPort, highPort;
		RecvHandlerParam::EventHandler onConnect, onMessage, onDisconnect;
		void *funcParam;
		std::size_t recvBufLen;

		//recvThread parameter associated with the current recvThread
		Rain::RecvHandlerParam rParam;

		//event handlers for internal recvThread, before passing to delegates
		static int onRecvInit(void *param);
		static int onRecvExit(void *param);
		static int onProcessMessage(void *param);
	};
}