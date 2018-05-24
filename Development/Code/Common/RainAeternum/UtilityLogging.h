/*
Standard
*/

/*
Functions to make program logging easier.
*/

#pragma once

#include "NetworkSocketManager.h"
#include "UtilityFilesystem.h"

#include <iostream>
#include <mutex>

namespace Rain {
	//thread-safe versatile class that can input from stdin, stdout, strings and sockets, and output to stdout and files
	//one RainLogger can IO from multiple sources, but its log stream will be the same to all outputs; if multiple logs are needed, use multiple RainLoggers
	class RainLogger {
		public:
		//enable/disable logging of socket communications
		bool setSocketSrc(Rain::SocketManager *nsm, bool use);

		//enable/disable stdin logging source
		bool setStdinSrc(bool enable);

		//enable/disable stdout logging source
		bool setStdoutSrc(bool use);

		//output a string to logging outputs
		void logString(std::string *s);
		void logString(std::string s);

		//enable/disable log output file
		bool setFileDst(std::string path, bool use);

		//enable/disable stdout logging destination
		bool setStdoutDst(bool use);

		//set a truncate rule on stdout destination per log; 0 = don't truncate
		int setStdoutTruncate(int len);

		private:
		//
	};
	
	//returns a shared mutex which locks cout for functions later
	std::mutex &getCoutMutex();

	//idea to adjust cout for multithreaded applications partially from http://stackoverflow.com/questions/18277304/using-stdcout-in-multiple-threads
	std::ostream &streamOutOne(std::ostream &os);

	template <class A0, class ...Args>
	std::ostream &streamOutOne(std::ostream &os, const A0 &a0, const Args &...args) {
		os << a0;
		return streamOutOne(os, args...);
	}
	template <class ...Args>
	std::ostream &streamOut(std::ostream &os, const Args &...args) {
		return streamOutOne(os, args...);
	}

	//use this cout function if don't want multiple threads to interrupt each other's output to console
	//TS = thread-safe
	template <class ...Args>
	std::ostream &tsCout(const Args &...args) {
		std::lock_guard<std::mutex> m_cout(getCoutMutex());
		return streamOut(std::cout, args...);
	}

	//output to both a logging file and the console; can set logging file via optional parameter
	//truncate the console log if log is too long
	//by defualt, truncate to one line; 0 doesn't truncate
	void outLogStdTruncRef(std::string &info, int maxLen = 80, std::string filePath = "", bool append = true);
	void outLogStdTrunc(std::string info, int maxLen = 80, std::string filePath = "", bool append = true);
	void outLogStd(std::string info, std::string filePath = "", bool append = true);

	//dumps memory leaks to a file if on debug mode; application must CloseHandle the return HANDLE, unless it's debug mode and the return is NULL
	HANDLE logMemoryLeaks(std::string out_file);
}