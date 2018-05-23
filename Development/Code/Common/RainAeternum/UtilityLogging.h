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
	class RainLogger {
		public:
		//add/remove stdin logging source
		static void setStdinSrc(bool use);

		//add/remove stdout logging source
		static void setStdoutSrc(bool use);

		//adds/removes logging of socket communications
		static void setNSMSrc(NetworkSocketManager &nsm, bool use);

		//output a string to logging outputs
		static void logString(std::string *s);
		static void logString(std::string s);

		//add/remove stdout logging destination
		static void setStdoutDst(bool use);

		//add/remove log output file
		static void setFileDst(std::string path, bool use);

		//set a truncate rule on stdout destination; 0 = don't truncate
		static void setStdoutTruncate(int len);

		private:
		
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