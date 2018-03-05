/*
Standard
*/

#pragma once
#include <string>
#include <iostream>
#include <fstream>

namespace Rain
{
	int ReportError (int code, std::string desc = "");
	std::streambuf *RedirectCerrFile (std::string filename);
}