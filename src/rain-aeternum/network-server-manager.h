/*
Standard
*/

/*
Implements thread for listening for socket connections and spawning recvThreads.
*/

#pragma once

#include "utility-error.h"
#include "utility-logging.h"
#include "network-base.h"
#include "network-recv-thread.h"
#include "network-socket-manager.h"
#include "network-utility.h"

#include <cstddef>
#include <mutex>

namespace Rain {
	//manages a single server/client connection, from the server-end
	class ServerSocketManager : public SocketManager {
		public:
		//parameter passed to delegate handlers
		struct ServerSocketManagerDelegateHandlerParam {
			//this object
			ServerSocketManager *ssm;

			//socket of the current connection
			SOCKET *cSocket;

			//message related to current event
			std::string *message;

			//additional parameters set by the creater of the server
			void *callerParam;

			//parameters unique to each ServerSocketManager, which is NULL by default and to be used by the event handlers
			void *delegateParam;
		};

		//ServerSocketManager must be initialized with a ServerManager parent and default event handlers set by the ServerManager's setEventHandlers
		ServerSocketManager(SOCKET *cSocket,
							RecvHandlerParam::EventHandler onConnect, 
							RecvHandlerParam::EventHandler onMessage, 
							RecvHandlerParam::EventHandler onDisconnect, 
							void *funcParam);
		~ServerSocketManager();

		//sends raw message over socket, without queueing or blocking
		void sendRawMessage(std::string request);
		void sendRawMessage(std::string *request);

		//returns current socket immediately
		SOCKET &getSocket();

		//set the delegate event handlers
		void setEventHandlers(RecvHandlerParam::EventHandler onConnect,
							  RecvHandlerParam::EventHandler onMessage,
							  RecvHandlerParam::EventHandler onDisconnect,
							  void *funcParam);

		//inheirited from SocketManager; sets logging on and off for communications on this socket; pass NULL to disable
		bool setLogging(void *logger);

		//gets the internal handlers, mainly for the use of the ServerManager
		//TODO: fix this
		std::tuple<RecvHandlerParam::EventHandler, RecvHandlerParam::EventHandler, RecvHandlerParam::EventHandler> getInternalHandlers();

		private:
		SOCKET *socket;
		LogStream *logger;

		//parameter to be passed to delegates
		ServerSocketManagerDelegateHandlerParam ssmdhParam;

		//recvThread parameter associated with the current recvThread
		RecvHandlerParam::EventHandler onConnectDelegate, onMessageDelegate, onDisconnectDelegate;

		//event handlers for recvThreads spwaned by ServerManager, which will call their delegates set by setEventHandlers of either ServerManager or ServerSocketManager
		//responsible for managing linked list created in ServerManager
		static int onConnect(void *param);
		static int onMessage(void *param);
		static int onDisconnect(void *param);
	};

	//spawns a ServerSocketManager for each new connection
	//A single server should accept a single type of client
	class ServerManager {
		public:
		typedef void(*NewClientFunc)(ServerSocketManager *);

		//passed to SM's event handlers
		//also used as linked list node for all spawned ServerSocketManagers
		struct ServerManagerRecvThreadParam {
			//used to call the event handler delegates in the SSM
			ServerSocketManager *ssm;

			//wraps SSM in a linked list node
			ServerManagerRecvThreadParam *prev, *next;

			//locked when changing linked list
			std::mutex *llMutex;

			//storage for the message of this recvThread
			std::string message;

			//handle to the current recvThread for use when waiting on end
			HANDLE hRecvThread;

			//pointer to the rhParam which has this object as its funcParam, so that the handler can free the recvParam
			Rain::RecvHandlerParam *rhParam;
		};

		ServerManager();
		~ServerManager();

		//returns current socket immediately
		SOCKET &getSocket();

		//returns current listening port #
		//returns -1 if not yet listening
		DWORD getListeningPort();

		//change socket to listen on different ports, or start socket listen on some ports
		//returns zero if no error
		//pass highPort = 0 to stop server listening
		int setServerListen(DWORD lowPort, DWORD highPort);

		//set the function to call when a new client is connected
		void setNewClientFunc(NewClientFunc newClientCall);

		//set the default delegate event handler for spawned recvThreads
		void setEventHandlers(RecvHandlerParam::EventHandler onConnect,
							  RecvHandlerParam::EventHandler onMessage,
							  RecvHandlerParam::EventHandler onDisconnect,
							  void *funcParam);

		//sets buffer length of spawned recvThreads
		std::size_t setRecvBufLen(std::size_t newLen);

		private:
		SOCKET socket;
		DWORD listeningPort;
		DWORD lowPort, highPort;
		NewClientFunc newClientCall;
		std::size_t recvBufLen;
		HANDLE ltEvent;

		//default handlers and params which ServerSocketManager should use
		RecvHandlerParam::EventHandler onConnectDelegate, onMessageDelegate, onDisconnectDelegate;
		void *funcParam;

		void disconnectSocket();

		//creates thread which accepts on a specified listening socket, until the socket is closed from outside the thread
		//accepted connections will spawn a recvThread for each connection with specified parameters
		//any parameters that the onProc functions will need to use will need to be created in onConnect - there will be space to store the parameters to be passed on the functions through the use of a void * in ListenThreadRecvParam
		//when listening socket is closed, thread will close all connection sockets and wait until all spawned recvThreads exit
		static DWORD WINAPI listenThread(LPVOID lpParameter);
	};
}