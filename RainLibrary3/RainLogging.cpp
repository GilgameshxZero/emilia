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
}