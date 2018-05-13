#include "RainLogging.h"

namespace Rain {
	const StreamFuncPtr endl = std::endl<char, std::char_traits<char> >;
	MultithreadedOStream rainCout(std::cout);

	MultithreadedOStream::MultithreadedOStream(std::ostream &orig_stream)
		:std::ostream(orig_stream.rdbuf()) {
	}

	MultithreadedOStream &MultithreadedOStream::operator<< (StreamFuncPtr manip) {
		std::lock_guard<std::mutex> m_l_g(m_stream);
		manip(*this);
		return *this;
	}

	std::mutex &MultithreadedOStream::GetMutex() {
		return this->m_stream;
	}

	std::ostream &streamOutOne(std::ostream &os) {
		return os;
	}

	std::mutex &getCoutMutex() {
		return rainCout.GetMutex();
	}

	void outLogStdTruncRef(std::string &info, int maxLen, std::string filePath, bool append) {
		static std::string persistentLogFilePath = "";
		if (filePath != "")
			persistentLogFilePath = filePath;
		Rain::fastOutputFileRef(persistentLogFilePath, info, append);
		if (maxLen != 0)
			Rain::rainCoutF(info.substr(0, maxLen));
		else
			Rain::rainCoutF(info);
	}

	void outLogStdTrunc(std::string info, int maxLen, std::string filePath, bool append) {
		Rain::outLogStdTruncRef(info, maxLen, filePath, append);
	}
}