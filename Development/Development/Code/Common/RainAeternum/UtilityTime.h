/*
Standard
*/

/*
Utility functions related to time formats.
*/

#pragma once

#include <string>
#include <time.h>

namespace Rain {
	std::string getTime(std::string format = "%D.%T%z");
}