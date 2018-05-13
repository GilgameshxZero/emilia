/*
Standard
*/

#pragma once

#include "RainUtilityFile.h"

#include <iostream>
#include <mutex>

namespace Rain {
	typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
	typedef CoutType &(*StreamFuncPtr)(CoutType &);

	/*
	idea partially from http://stackoverflow.com/questions/1134388/stdendl-is-of-unknown-type-when-overloading-operator
	allows for extension of << operator of any stream to not have race conditions
	doesn't guarantee << to be in order (particularily, a chain of << statements is not guaranteed to execute in order), but makes it much more likely
	for a guarantee, use rainCoutF
	*/
	class MultithreadedOStream : std::ostream {
		public:
		MultithreadedOStream(std::ostream &orig_stream);

		template <typename T>
		MultithreadedOStream &operator<< (const T &x) {
			std::lock_guard<std::mutex> m_l_g(m_stream);
			//std::ostream::operator<< (x); doesn't work as expected
			static_cast<std::ostream &>(*this) << x;
			return *this;
		}

		//function for std::endl
		MultithreadedOStream &operator<< (StreamFuncPtr manip);

		std::mutex &GetMutex();

		private:
		//one mutex for each MultithreadedOStream object
		std::mutex m_stream;
	};

	//use Rain::endl instead of std::endl when using rainCoutF
	extern const StreamFuncPtr endl;

	//use rainCout instead of std::cout
	extern MultithreadedOStream rainCout;

	//idea to adjust cout for multithreaded applications partially from http://stackoverflow.com/questions/18277304/using-stdcout-in-multiple-threads
	std::ostream &streamOutOne(std::ostream &os);
	std::mutex &getCoutMutex();

	template <class A0, class ...Args>
	std::ostream &streamOutOne(std::ostream &os, const A0 &a0, const Args &...args) {
		os << a0;
		return streamOutOne(os, args...);
	}

	template <class ...Args>
	std::ostream &streamOut(std::ostream &os, const Args &...args) {
		return streamOutOne(os, args...);
	}

	template <class ...Args>
	std::ostream &rainCoutF(const Args &...args) {
		std::lock_guard<std::mutex> m_cout(getCoutMutex());
		return streamOut(std::cout, args...);
	}

	//output to both a logging file and the console; can set logging file via optional parameter
	//truncate the console log if log is too long
	//by defualt, truncate to one line; 0 doesn't truncate
	void outLogStdTruncRef(std::string &info, int maxLen = 80, std::string filePath = "", bool append = true);
	void outLogStdTrunc(std::string info, int maxLen = 80, std::string filePath = "", bool append = true);
}