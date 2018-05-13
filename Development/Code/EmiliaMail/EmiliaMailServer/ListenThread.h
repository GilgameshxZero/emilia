/*
Responsible for maintaining a socket open for listening on a specified port, as well as spawning sockets for every new connection, and calling functions to deal with messages once they are passed in.
*/

#pragma once

#include "../../Common/RainLibrary3/RainLibraries.h"
#include "ListenThreadParam.h"
#include "RecvThreadParam.h"
#include "RecvThreadHandlers.h"

#include <iomanip>
#include <fstream>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <Windows.h>

namespace Monochrome3 {
	namespace EMTSMTPServer {
		//called from Start to create a thread
		DWORD WINAPI listenThread(LPVOID lpParameter);

		//message handlers for the listenThread message queue/window
		LRESULT onListenWndEnd(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		LRESULT onListenWndInit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	}
}