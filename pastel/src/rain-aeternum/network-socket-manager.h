/*
Standard
*/

/*
Defines abstract class which communicates over an already-connected socket, and allows for communications to be logged.
*/

#pragma once

#include <string>

namespace Rain {
	class SocketManager {
	public:
		virtual void sendRawMessage(std::string request) = 0;
		virtual void sendRawMessage(std::string *request) = 0;

		//implementations of SocketManager should include UtilityLogging to convert logger into a type LogStream *, and call logString when necessary
		virtual bool setLogging(void *logger) = 0;

		//wait until timeout elapses or queued messages are sent
		//0 for infinite
		virtual void blockForMessageQueue(DWORD msTimeout = 5000) = 0;
	};
}