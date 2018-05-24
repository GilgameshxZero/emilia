#include "UtilityLogging.h"

namespace Rain {
	RainLogger::RainLogger() {
		this->outputStdout = false;
		this->stdoutTrunc = 0;

		this->stdinThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		this->stdoutThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	RainLogger::~RainLogger() {
		//terminate any threads
		SetEvent(this->stdinThreadEvent);
		SetEvent(this->stdoutThreadEvent);
	}
	void RainLogger::setSocketSrc(Rain::SocketManager *nsm, bool enable) {
		nsm->setLogging(enable ? reinterpret_cast<void *>(nsm) : NULL);
	}
	void RainLogger::setStdinSrc(bool enable) {
		//start thread to capture stdin
		if (enable) {
			ResetEvent(this->stdoutThreadEvent);
			CreateThread(NULL, 0, this->stdioLogThread, reinterpret_cast<LPVOID>(new std::pair<RainLogger *, HANDLE>(this, this->stdoutThreadEvent)), 0, NULL);
		} else
			SetEvent(this->stdoutThreadEvent);
	}
	void RainLogger::setStdoutSrc(bool enable) {
		//start thread to capture stdout
		if (enable) {
			ResetEvent(this->stdoutThreadEvent);
			CreateThread(NULL, 0, this->stdioLogThread, reinterpret_cast<LPVOID>(new std::pair<RainLogger *, HANDLE>(this, this->stdinThreadEvent)), 0, NULL);
		} else
			SetEvent(this->stdoutThreadEvent);
	}
	void RainLogger::logString(std::string *s) {
		static std::string header;
		header = Rain::getTime() + " " + Rain::tToStr(s->length()) + " bytes\r\n";
		if (this->outputStdout)
			std::cout << header
			<< s->substr(0, this->stdoutTrunc == 0 ? std::string::npos : this->stdoutTrunc)
			<< "\r\n\r\n";
		for (std::string file: this->fileDst) {
			Rain::printToFile(file, header, true);
			Rain::printToFile(file, s, true);
			Rain::printToFile(file, "\r\n\r\n", true);
		}
	}
	void RainLogger::logString(std::string s) {
		this->logString(&s);
	}
	void RainLogger::setFileDst(std::string path, bool enable) {
		if (enable)
			this->fileDst.insert(path);
		else
			this->fileDst.erase(path);
	}
	void RainLogger::setStdoutDst(bool enable, std::size_t len) {
		this->outputStdout = enable;
		this->stdoutTrunc = len;
	}
	DWORD WINAPI RainLogger::stdioLogThread(LPVOID lpParameter) {
		std::pair<RainLogger *, HANDLE> &param = *reinterpret_cast<std::pair<RainLogger *, HANDLE> *>(lpParameter);
		RainLogger &logger = *param.first;
		HANDLE breakEvent = param.second,
			hSrc = param.second == logger.stdinThreadEvent ? GetStdHandle(STD_INPUT_HANDLE) : GetStdHandle(STD_OUTPUT_HANDLE);

		HANDLE hCommandWait[2] = {breakEvent, hSrc};

		while (true) {
			static std::string command, cmdLeftover;
			static DWORD waitRtrn;
			waitRtrn = WaitForMultipleObjects(2, hCommandWait, FALSE, INFINITE);
			if (waitRtrn == WAIT_OBJECT_0) {
				break;
			} else if (waitRtrn == WAIT_OBJECT_0 + 1) {
				//get user input from low-level console functions
				static DWORD numberOfEvents;
				GetNumberOfConsoleInputEvents(hCommandWait[1], &numberOfEvents);

				if (numberOfEvents > 0) {
					static INPUT_RECORD *inputs;
					static DWORD eventsRead;
					inputs = new INPUT_RECORD[numberOfEvents];
					ReadConsoleInput(hCommandWait[1], inputs, numberOfEvents, &eventsRead);
					for (int a = 0; a < static_cast<int>(numberOfEvents); a++) {
						if (inputs[a].EventType == KEY_EVENT &&
							inputs[a].Event.KeyEvent.bKeyDown == TRUE) {
							command += inputs[a].Event.KeyEvent.uChar.AsciiChar;

							//TODO: pressing enter only inputs \r for some reason, so append \n if that's the case
							if (inputs[a].Event.KeyEvent.uChar.AsciiChar == '\r')
								command += '\n';
						}
					}
					delete[] inputs;

					//check if there's a newline somewhere
					static size_t newlinePos;
					newlinePos = command.find('\n');
					if (newlinePos != std::string::npos) {
						logger.logString(command);
						command.clear();
					}
				}
			}
		}

		delete &param;
		return 0;
	}

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
			HANDLE mem_leak = CreateFile(out_file.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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