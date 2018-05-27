#include "UtilityLogging.h"

namespace Rain {
	LogStream::LogStream() {
		this->outputStdout = false;
		this->stdoutTrunc = 0;
	}
	LogStream::~LogStream() {
		//terminate any threads
		for (auto it : this->stdPipeSrc) {
			this->setStdHandleSrc(it.first, false);
		}
	}
	void LogStream::setSocketSrc(Rain::SocketManager *nsm, bool enable) {
		nsm->setLogging(enable ? reinterpret_cast<void *>(nsm) : NULL);
	}
	bool LogStream::setStdHandleSrc(DWORD stdHandle, bool enable) {
		bool ret = this->stdPipeSrc.find(stdHandle) != this->stdPipeSrc.end();
		int stdFileNo;
		if (stdHandle == STD_OUTPUT_HANDLE)
			stdFileNo = _fileno(stdout);
		else if (stdHandle == STD_INPUT_HANDLE)
			stdFileNo = _fileno(stdin);
		else
			stdFileNo = _fileno(stderr);
		if (enable && !ret) {
			//redirect stdin to a pipe, then from that pipe to the original stdin pipe
			HANDLE rd, wr;
			CreatePipe(&rd, &wr, NULL, 0);
			int fdDup = _dup(stdFileNo),
				wrOsHandle = _open_osfhandle(reinterpret_cast<intptr_t>(wr), 0);
			HANDLE hOrig = reinterpret_cast<HANDLE>(_get_osfhandle(fdDup)); //closed in thread
			_dup2(wrOsHandle, stdFileNo);

			//save handles to a map, to access later when the stdsrc is disabled
			this->stdPipeSrc.insert(std::make_pair(stdHandle, std::make_tuple(
				rd, 
				wr, 
				fdDup, 
				wrOsHandle, 
				CreateThread(NULL, 0, this->pipeRedirectThread, reinterpret_cast<LPVOID>(new std::tuple<HANDLE, HANDLE, LogStream *>(
					rd, 
					hOrig, 
					this)), 0, NULL))));
		} else if (!enable && ret) {
			auto elem = this->stdPipeSrc.find(stdHandle)->second;
			HANDLE rd = std::get<0>(elem),
				wr = std::get<1>(elem);
			int fdDup = std::get<2>(elem),
				wrOsHandle = std::get<3>(elem);
			HANDLE hThread = std::get<4>(elem);

			_dup2(fdDup, stdFileNo);
			_close(wrOsHandle);

			//stop logging this pipe and terminate corresponding thread
			this->stdPipeSrc.erase(stdHandle);
			CloseHandle(wr);
			CloseHandle(rd);
			CancelSynchronousIo(hThread);
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
		return ret;
	}
	void LogStream::logString(std::string *s) {
		static std::string header;
		header = Rain::getTime() + " " + Rain::tToStr(s->length()) + "B\r\n";
		if (this->outputStdout) {
			//shutdown stdout logging before doing cout, then turn it on again if it was on before
			bool origValue = this->setStdHandleSrc(STD_OUTPUT_HANDLE, false);
			Rain::tsCout(header, s->substr(0, this->stdoutTrunc == 0 ? std::string::npos : this->stdoutTrunc), "\r\n\r\n");
			this->setStdHandleSrc(STD_OUTPUT_HANDLE, origValue);
		}
		for (std::string file: this->fileDst) {
			Rain::printToFile(file, header, true);
			Rain::printToFile(file, s, true);
			Rain::printToFile(file, "\r\n\r\n", true);
		}
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
	DWORD WINAPI LogStream::pipeRedirectThread(LPVOID lpParameter) {
		static const std::size_t bufSize = 65536;
		std::tuple<HANDLE, HANDLE, LogStream *> &param = *reinterpret_cast<std::tuple<HANDLE, HANDLE, LogStream *> *>(lpParameter);
		HANDLE rd = std::get<0>(param), wr = std::get<1>(param);
		LogStream &logger = *std::get<2>(param);

		DWORD dwRead, dwWritten;
		CHAR chBuf[bufSize];
		BOOL bSuccess = FALSE;
		std::string mAcc;

		while (true) {
			bSuccess = ReadFile(rd, chBuf, bufSize, &dwRead, NULL);
			if (!bSuccess || dwRead == 0) break; //if synchronious io is closed, we will break and exit thread
			bSuccess = WriteFile(wr, chBuf, dwRead, &dwWritten, NULL);
			if (!bSuccess) break;

			//collect pipe ins for enter
			mAcc += std::string(chBuf, dwRead);

			static size_t newlinePos;
			newlinePos = mAcc.find('\n');
			if (newlinePos != std::string::npos) {
				logger.logString(&mAcc);
				mAcc.clear();
			}
		}

		//_close(reinterpret_cast<intptr_t>(wr));
		//delete lpParameter;
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