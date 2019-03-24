#include "utility-libraries.h"

namespace Rain {
	HANDLE simpleCreateThread(LPTHREAD_START_ROUTINE threadfunc, LPVOID threadparam) {
		std::thread newThread(threadfunc, threadparam);
		newThread.detach();
		return newThread.native_handle();
	}

	COLORREF colorFromHex(std::string hex) {
		return RGB(static_cast<unsigned char>(hexToChr(std::make_pair(hex[0], hex[1]))),
			static_cast<unsigned char>(hexToChr(std::make_pair(hex[2], hex[3]))),
			static_cast<unsigned char>(hexToChr(std::make_pair(hex[4], hex[5]))));
	}
}  // namespace Rain