#include "utility-logging.h"

namespace Rain {
	LogStream::LogStream() {
		this->outputStdout = false;
		this->stdoutTrunc = 0;
	}
	LogStream::~LogStream() {
		//terminate any threads
		for (auto it : this->stdSrcMap) {
			this->setStdHandleSrc(it.first, false);
		}
	}
	void LogStream::setSocketSrc(Rain::SocketManager *nsm, bool enable) {
		nsm->setLogging(enable ? reinterpret_cast<void *>(this) : NULL);
	}
	bool LogStream::setStdHandleSrc(DWORD stdHandle, bool enable) {
		bool ret = this->stdSrcMap.find(stdHandle) != this->stdSrcMap.end();
		FILE *stdFilePtr;
		if (stdHandle == STD_OUTPUT_HANDLE)
			stdFilePtr = stdout;
		else if (stdHandle == STD_INPUT_HANDLE)
			stdFilePtr = stdin;
		else
			stdFilePtr = stderr;
		if (enable && !ret) {
			//redirect stdin to a pipe, then from that pipe to the original stdin pipe
			HANDLE rd, wr;
			CreatePipe(&rd, &wr, NULL, 0);
			int oshOrigStdSrc = _dup(_fileno(stdFilePtr)),
				oshRepPipeWr = _open_osfhandle(reinterpret_cast<intptr_t>(wr), 0);
			HANDLE hOrig = reinterpret_cast<HANDLE>(_get_osfhandle(oshOrigStdSrc));
			_dup2(oshRepPipeWr, _fileno(stdFilePtr));

			//save handles to a map, to access later when the stdsrc is disabled
			StdSrcInfo &ssi = *this->stdSrcMap.insert(std::make_pair(stdHandle, new StdSrcInfo())).first->second;
			ssi.thParam.logger = this;
			ssi.thParam.rd = rd;
			ssi.thParam.wr = hOrig;
			ssi.thParam.running = true;

			ssi.oshOrigStdSrc = oshOrigStdSrc;
			ssi.oshRepPipeWr = oshRepPipeWr;
			ssi.repPipeRd = rd;
			ssi.repPipeWr = wr;
			ssi.hThread = CreateThread(NULL, 0, this->stdSrcRedirectThread, reinterpret_cast<LPVOID>(&ssi.thParam), 0, NULL);
		} else if (!enable && ret) {
			fflush(stdFilePtr);

			//get params
			StdSrcInfo &ssi = *this->stdSrcMap.find(stdHandle)->second;

			//stop logging this pipe and terminate corresponding thread
			ssi.thParam.running = false;
			CancelSynchronousIo(ssi.hThread);
			WaitForSingleObject(ssi.hThread, INFINITE);
			_close(ssi.oshRepPipeWr); //calls CloseHandle on ssi.repPipeWr
			CloseHandle(ssi.repPipeRd);
			CloseHandle(ssi.hThread);

			//reset std handles and free memory
			this->stdSrcMap.erase(stdHandle); //erase here, so that we can log everything in stdout if we are logging to stdout as well
			_dup2(ssi.oshOrigStdSrc, _fileno(stdFilePtr));
			delete &ssi;
		}
		return ret;
	}
	void LogStream::logString(std::string *s) {
		//logging will be thread-safe
		static std::mutex logMutex;

		static std::string header;

		logMutex.lock();

		header = Rain::getTime() + " " + Rain::tToStr(s->length()) + "\r\n";
		if (this->outputStdout) {
			//if STD_OUTPUT_HANDLE is being logged, then temporarily replace with original output handles while outputting log, so that we don't log indefinitely
			auto itMap = this->stdSrcMap.find(STD_OUTPUT_HANDLE);
			if (itMap != this->stdSrcMap.end()) {
				fflush(stdout);
				_dup2(itMap->second->oshOrigStdSrc, _fileno(stdout));
			}
			Rain::tsCout(header, s->substr(0, this->stdoutTrunc == 0 ? std::string::npos : this->stdoutTrunc), "\r\n\r\n");
			if (itMap != this->stdSrcMap.end()) {
				fflush(stdout);
				_dup2(itMap->second->oshRepPipeWr, _fileno(stdout));
			}
		}
		for (std::string file : this->fileDst) {
			Rain::printToFile(file, &header, true);
			Rain::printToFile(file, s, true);
			Rain::printToFile(file, "\r\n\r\n", true);
		}

		logMutex.unlock();
	}
	void LogStream::logString(std::string s) {
		this->logString(&s);
	}
	void LogStream::setFileDst(std::string path, bool enable) {
		if (enable)
			this->fileDst.insert(path);
		else
			this->fileDst.erase(path);
	}
	void LogStream::setStdoutDst(bool enable, std::size_t len) {
		this->outputStdout = enable;
		this->stdoutTrunc = len;
	}
	DWORD WINAPI LogStream::stdSrcRedirectThread(LPVOID lpParameter) {
		static const std::size_t bufSize = 65536;
		StdSrcRedirectThreadParam &param = *reinterpret_cast<StdSrcRedirectThreadParam *>(lpParameter);

		DWORD dwRead, dwWritten;
		CHAR chBuf[bufSize];
		BOOL bSuccess = FALSE;
		std::string message;

		while (param.running) {
			bSuccess = ReadFile(param.rd, chBuf, bufSize, &dwRead, NULL); //blocks until cancelled or read
			if (!bSuccess || dwRead == 0) break;
			param.logger->logString(std::string(chBuf, dwRead)); //log here before console in case of error
			bSuccess = WriteFile(param.wr, chBuf, dwRead, &dwWritten, NULL);
			if (!bSuccess) break; //unexpected error
		}

		//if we're here, read what's remaining in the pipe if possible before exiting
		DWORD bytesAvail;
		PeekNamedPipe(param.rd, NULL, 0, NULL, &bytesAvail, NULL);
		while (bytesAvail > 0) { //the same procedure as the rest of the loop
			ReadFile(param.rd, chBuf, bufSize, &dwRead, NULL); //don't check for error, since synchronious io is already cancelled, so this will probably error
			param.logger->logString(std::string(chBuf, dwRead));
			if (!WriteFile(param.wr, chBuf, dwRead, &dwWritten, NULL)) break;
			PeekNamedPipe(param.rd, NULL, 0, NULL, &bytesAvail, NULL);
		}

		return 0;
	}

	std::mutex &getCoutMutex() {
		static std::mutex coutMutex;
		return coutMutex;
	}

	std::ostream &streamOutOne(std::ostream &os) {
		return os;
	}

	void outLogStdTrunc(std::string *info, int maxLen, std::string filePath, bool append) {
		static std::string persistentLogFilePath = "";
		if (filePath != "")
			persistentLogFilePath = filePath;
		Rain::printToFile(persistentLogFilePath, info, append);
		if (maxLen != 0)
			Rain::tsCout(info->substr(0, maxLen));
		else
			Rain::tsCout(*info);
	}
	void outLogStdTrunc(std::string info, int maxLen, std::string filePath, bool append) {
		Rain::outLogStdTrunc(&info, maxLen, filePath, append);
	}
	void outLogStd(std::string info, std::string filePath, bool append) {
		Rain::outLogStdTrunc(&info, 0, filePath, append);
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