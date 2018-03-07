/*
Standard
*/

#pragma once

#include "RainWindowsLAM.h"

#include <string>
#include <time.h>

namespace Rain {
	HANDLE simpleCreateThread(LPTHREAD_START_ROUTINE threadfunc, LPVOID threadparam);

	//dumps memory leaks to a file if on debug mode; application must CloseHandle the return HANDLE, unless it's debug mode and the return is NULL
	HANDLE logMemoryLeaks(std::string out_file);

	std::string getTime(std::string format = "%Y-%m-%d.%X");
}