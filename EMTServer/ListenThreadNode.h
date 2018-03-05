#pragma once

#include "../RainLibrary3/RainLibraries.h"

struct CTLLNode {
	CTLLNode *prev, *next;
	HWND hwnd; //message queue
};