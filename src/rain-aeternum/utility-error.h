/*
Standard
*/

#pragma once

#include "utility-logging.h"

#include <fstream>
#include <iostream>
#include <string>

namespace Rain {
	int reportError(int code, std::string desc = "");
	int errorAndCout(int code, std::string desc = "", std::string endl = CRLF);
	std::pair<std::streambuf *, std::ofstream *> redirectCerrFile(std::string filename, bool append = false);
}