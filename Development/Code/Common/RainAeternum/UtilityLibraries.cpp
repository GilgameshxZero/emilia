#include "UtilityLibraries.h"

namespace Rain {
	HANDLE simpleCreateThread(LPTHREAD_START_ROUTINE threadfunc, LPVOID threadparam) {
		return CreateThread(NULL, 0, threadfunc, threadparam, 0, NULL);
	}
}