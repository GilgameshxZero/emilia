#pragma once

#include "ListenThreadNode.h"

//parameter passed to any listening thread
struct ListenThreadParam {
	//persistent socket on which to listen for clients
	SOCKET *lSocket;

	//mutex to lock the link list when modifications are being made to it
	std::mutex *ltLLMutex;

	//parameters to pass to message procs
	std::string serverRootDir;

	CTLLNode *beg, *end; //a linked list with all the client thread information
						 //beg points to a valid node, with prev = NULL, or NULL if the LL is empty
						 //end points to a valid node, with next = NULL, or NULL if the LL is empty
};