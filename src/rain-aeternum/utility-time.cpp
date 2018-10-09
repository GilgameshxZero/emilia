#include "utility-time.h"

namespace Rain {
	std::string getTime(std::string format) {
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), format.c_str(), &tstruct);
		return buf;
	}
}