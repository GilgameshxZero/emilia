/*
Standard
*/

#pragma once

#include "utility-string.h"

namespace Rain {
	//retreive the status code from a message, such as 250 or 221
	//returns -1 for parsing error
	int getSMTPStatus(std::string *message);
	int getSMTPStatus(std::string message);

	//gets email parts from a string
	std::string getEmailDomain(std::string email);
	std::string getEmailUser(std::string email);
}