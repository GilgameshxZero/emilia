#include "RainUtilityGeneral.h"

namespace Rain {
	HANDLE simpleCreateThread(LPTHREAD_START_ROUTINE threadfunc, LPVOID threadparam) {
		return CreateThread(NULL, 0, threadfunc, threadparam, 0, NULL);
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
	
	std::string getTime(std::string format) {
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), format.c_str(), &tstruct);
		return buf;
	}
}