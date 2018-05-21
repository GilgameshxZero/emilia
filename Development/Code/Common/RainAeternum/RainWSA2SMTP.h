/*
Standard
*/

#pragma once

#include "RainUtilityString.h"

#include <string>

namespace Rain {
	//retreive the status code from a message, such as 250 or 221
	//returns -1 for parsing error
	int getSMTPStatusCode(std::string &message);
}