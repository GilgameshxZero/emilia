/*
Standard
*/

/*
Include this for all WinAPI Winsock2 libraries, instead of winsock2 directly.
*/

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#pragma comment(lib, "Ws2_32.lib")

#include "windows-lam-include.h"

#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>