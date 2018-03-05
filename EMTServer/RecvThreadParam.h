#pragma once

#include "ListenThreadParam.h"

#include <queue>

struct RecvThreadParam {
	std::string serverRootDir;
	std::string serverAux;

	ListenThreadParam *ctllnode;
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