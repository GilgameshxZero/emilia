/*
Standard
*/

/*
Include this for all UtilityLibraries libraries.
*/

#pragma once

#include "UtilityFilesystem.h"
#include "UtilityString.h"
#include "UtilityTime.h"

namespace Rain {
	HANDLE simpleCreateThread(LPTHREAD_START_ROUTINE threadfunc, LPVOID threadparam);
}