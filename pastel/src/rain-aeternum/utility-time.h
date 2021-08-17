/*
Standard
*/

/*
Utility functions related to time formats.
*/

#pragma once

#include <string>
#include <time.h>
#include <thread>

namespace Rain {
	std::string getTime(std::string format = "%D.%T%z", time_t now = time(0));

	void sleep(int ms);
}