/*
Standard
*/

#pragma once

#include <string>
#include <fstream>

namespace Rain
{
	void OutputFile (std::string filename, std::string &output, bool append = false);
}