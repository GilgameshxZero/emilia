/*
Standard
*/

#pragma once

#include <string>

namespace Rain {
	class NetworkSocketManager {
		public:
		virtual void sendRawMessage(std::string request) = 0;
		virtual void sendRawMessage(std::string *request) = 0;
	};
}