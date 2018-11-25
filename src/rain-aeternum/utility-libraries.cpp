#include "utility-libraries.h"

namespace Rain {
	HANDLE simpleCreateThread(LPTHREAD_START_ROUTINE threadfunc, LPVOID threadparam) {
		return CreateThread(NULL, 0, threadfunc, threadparam, 0, NULL);
	}

	COLORREF colorFromHex(std::string hex) {
		return RGB(static_cast<unsigned char>(hexToChr(std::make_pair(hex[0], hex[1]))),
				   static_cast<unsigned char>(hexToChr(std::make_pair(hex[2], hex[3]))),
				   static_cast<unsigned char>(hexToChr(std::make_pair(hex[4], hex[5]))));
	}
}  // namespace Rain