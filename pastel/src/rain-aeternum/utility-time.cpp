#include "utility-time.h"

namespace Rain {
	std::string getTime(std::string format, time_t now) {
		struct tm  tstruct;
		char       buf[80];
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), format.c_str(), &tstruct);
		return buf;
	}

	void sleep(int ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
}