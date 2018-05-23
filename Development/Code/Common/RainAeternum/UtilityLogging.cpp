#include "UtilityLogging.h"

namespace Rain {
	std::ostream &streamOutOne(std::ostream &os) {
		return os;
	}

	std::mutex &getCoutMutex() {
		static std::mutex coutMutex;
		return coutMutex;
	}

	void outLogStdTruncRef(std::string &info, int maxLen, std::string filePath, bool append) {
		static std::string persistentLogFilePath = "";
		if (filePath != "")
			persistentLogFilePath = filePath;
		Rain::printToFile(persistentLogFilePath, &info, append);
		if (maxLen != 0)
			Rain::tsCout(info.substr(0, maxLen));
		else
			Rain::tsCout(info);
	}
	void outLogStdTrunc(std::string info, int maxLen, std::string filePath, bool append) {
		Rain::outLogStdTruncRef(info, maxLen, filePath, append);
	}
	void outLogStd(std::string info, std::string filePath, bool append) {
		Rain::outLogStdTruncRef(info, 0, filePath, append);
	}

	HANDLE logMemoryLeaks(std::string out_file) {
		if (IsDebuggerPresent()) {
			//Redirect the error stream to a file, only if the program is debugging.
			HANDLE mem_leak = CreateFile(out_file.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			//Turn on debugging for memory leaks. This is automatically turned off when the build is Release.
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
			_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
			_CrtSetReportFile(_CRT_WARN, mem_leak);
			_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
			_CrtSetReportFile(_CRT_ERROR, mem_leak);
			_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
			_CrtSetReportFile(_CRT_ASSERT, mem_leak);

			return mem_leak;
		}

		return NULL;
	}
}