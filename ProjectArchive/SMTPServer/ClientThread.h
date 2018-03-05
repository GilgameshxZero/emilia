#pragma once
#include "RainWSA2.h"
#include "SendRecv.h"
#include "Utility.h"
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <Windows.h>

namespace SMTPS
{
	struct CTLLNode
	{
		CTLLNode *prev, *next;
		HWND hwnd; //message queue
	};

	struct ClientThreadParam
	{
		CTLLNode *beg, *end; //a linked list with all the client thread information
							 //beg points to a valid node, with prev = NULL, or NULL if the LL is empty
							 //end points to a valid node, with next = NULL, or NULL if the LL is empty
		std::mutex ctllmutex;
		SOCKET *listensock;
		HWND consolewnd;
		long long threadnum;
	};

	struct ClientProcParam
	{
		SOCKET *listensock, clientsock;
		Rain::RecvParam recvparam;
		HANDLE hrecvthread;
		ClientThreadParam *ctparam;
		CTLLNode *ctllnode;
	};

	struct RecvFuncParam
	{
		CTLLNode *ctllnode;
		std::queue<char> mqueue;
		SOCKET *sock;
		std::string message, fmess;

		//for processing POST
		std::map<std::string, std::string> headermap;
		std::string reqtype, requrl, httpver;
		bool waitingPOST;
		std::string POSTmessage;
		std::size_t POSTlen;
	};

	DWORD WINAPI ClientThreadProc (LPVOID lpParameter);
	LRESULT CALLBACK ClientWndProc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	int ProcClientMess (void *param);
	void OnClientRecvEnd (void *param);

	int ProcFullMess (RecvFuncParam *rfparam);
}