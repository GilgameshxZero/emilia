/*
Standard
*/

#pragma once

#include "UtilityString.h"

#include <string>

namespace Rain {
	//retreive the status code from a message, such as 250 or 221
	//returns -1 for parsing error
	int getSMTPStatus(std::string message);
	int getSMTPStatus(std::string *message);
}