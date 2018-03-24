/*
Standard
*/

#pragma once

#include <fstream>
#include <iostream>
#include <string>

namespace Rain {
	int reportError(int code, std::string desc = "");
	std::streambuf *redirectCerrFile(std::string filename, bool append = false);
}