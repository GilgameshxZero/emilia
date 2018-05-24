#include "UtilityFilesystem.h"

namespace Rain {
	bool fileExists(std::string file) {
		struct stat buffer;
		return (stat(file.c_str(), &buffer) == 0);
	}

	std::string getWorkingDirectory() {
		DWORD bufferLen = GetCurrentDirectory(0, NULL);
		TCHAR *buffer = new TCHAR[bufferLen];
		GetCurrentDirectory(bufferLen, buffer);
		std::string ret(buffer);
		delete[] buffer;
		return ret + "\\";
	}

	std::string getExePath() {
		char multibyte[MAX_PATH];
		wchar_t buffer[MAX_PATH];

		GetModuleFileNameW(NULL, buffer, MAX_PATH);
		WideCharToMultiByte(CP_UTF8, 0, buffer, -1, multibyte, MAX_PATH, NULL, NULL);
		return std::string(multibyte);
	}

	std::string getPathDir(std::string path) {
		return path.substr(0, path.find_last_of("\\/")) + "\\";
	}
	std::string getPathFile(std::string path) {
		return path.substr(path.find_last_of("\\/") + 1, path.length());
	}

	std::vector<std::string> getFiles(std::string directory, std::string format) {
		char search_path[MAX_PATH], multibyte[MAX_PATH];
		wchar_t unicodesp[MAX_PATH];
		sprintf_s(search_path, ("%s/" + format).c_str(), directory.c_str());
		WIN32_FIND_DATAW fd;

		MultiByteToWideChar(CP_UTF8, 0, search_path, -1, unicodesp, MAX_PATH);
		int k = GetLastError();
		HANDLE hFind = ::FindFirstFileW(unicodesp, &fd);

		std::vector<std::string> ret;
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, multibyte, MAX_PATH, NULL, NULL);
					ret.push_back(multibyte);
				}
			} while (::FindNextFileW(hFind, &fd));
			::FindClose(hFind);
		}

		return ret;
	}
	std::vector<std::string> getDirs(std::string directory, std::string format) {
		char search_path[MAX_PATH], multibyte[MAX_PATH];
		wchar_t unicodesp[MAX_PATH];
		sprintf_s(search_path, ("%s/" + format).c_str(), directory.c_str());
		WIN32_FIND_DATAW fd;

		MultiByteToWideChar(CP_UTF8, 0, search_path, -1, unicodesp, MAX_PATH);
		int k = GetLastError();
		HANDLE hFind = ::FindFirstFileW(unicodesp, &fd);

		std::vector<std::string> ret;
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, multibyte, MAX_PATH, NULL, NULL);
					ret.push_back(multibyte);
				}
			} while (::FindNextFileW(hFind, &fd));
			::FindClose(hFind);
		}

		return ret;
	}

	void createDirRec(std::string dir) {
		size_t pos = 0;
		dir = dir.substr(0, dir.length() - 1); //remove the '\\'
		do {
			pos = dir.find_first_of("\\/", pos + 1);
			CreateDirectory(dir.substr(0, pos).c_str(), NULL);
		} while (pos != std::string::npos);
	}

	void recursiveRmDir(std::string dir) {
		wchar_t unicode[MAX_PATH];
		std::vector<std::string> ldir, lfile;

		ldir = getDirs(dir, "*");
		lfile = getFiles(dir, "*");

		for (std::size_t a = 0; a < lfile.size(); a++) {
			MultiByteToWideChar(CP_UTF8, 0, (dir + lfile[a]).c_str(), -1, unicode, MAX_PATH);
			DeleteFileW(unicode);
		}
		for (std::size_t a = 2; a < ldir.size(); a++) //skip . and ..
		{
			MultiByteToWideChar(CP_UTF8, 0, (dir + ldir[a] + '\\').c_str(), -1, unicode, MAX_PATH);
			recursiveRmDir(dir + ldir[a] + '\\');
			RemoveDirectoryW(unicode);
		}
	}

	std::string pathToAbsolute(std::string path) {
		TCHAR tcFullPath[32767];
		GetFullPathName(path.c_str(), 32767, tcFullPath, NULL);
		return tcFullPath;
	}

	BY_HANDLE_FILE_INFORMATION getFileInformation(std::string path) {
		HANDLE handle = CreateFile(path.c_str(),
								   0,
								   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
								   NULL,
								   OPEN_EXISTING,
								   FILE_ATTRIBUTE_NORMAL,
								   NULL);
		BY_HANDLE_FILE_INFORMATION info;
		GetFileInformationByHandle(handle, &info);
		return info;
	}
	bool isEquivalentPath(std::string path1, std::string path2) {
		BY_HANDLE_FILE_INFORMATION info1 = getFileInformation(path1),
			info2 = getFileInformation(path2);
		return (info1.dwVolumeSerialNumber == info2.dwVolumeSerialNumber &&
				info1.nFileIndexLow == info2.nFileIndexLow &&
				info1.nFileIndexHigh == info2.nFileIndexHigh);
	}

	FILETIME getLastModTime(std::string path) {
		wchar_t unicode[MAX_PATH];
		FILETIME ret;

		MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, unicode, MAX_PATH);
		HANDLE hfile = CreateFileW(unicode, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hfile == INVALID_HANDLE_VALUE)
			ret.dwLowDateTime = ret.dwHighDateTime = 0;
		else {
			GetFileTime(hfile, NULL, NULL, &ret);
			CloseHandle(hfile);
		}

		return ret;
	}

	std::vector<std::string> getFilesRec(std::string directory, std::string format) {
		std::vector<std::string> dirs, files;
		dirs = Rain::getDirs(directory, "*");
		files = Rain::getFiles(directory, format);

		std::vector<std::string> relpath;
		for (std::size_t a = 0; a < files.size(); a++)
			relpath.push_back(files[a]);

		for (std::size_t a = 2; a < dirs.size(); a++) {
			std::vector<std::string> subrel;
			subrel = getFilesRec(directory + dirs[a] + "\\", format);

			for (std::size_t b = 0; b < subrel.size(); b++)
				relpath.push_back(dirs[a] + "\\" + subrel[b]);
		}
		return relpath;
	}
	std::vector<std::string> getDiresRec(std::string directory, std::string format) {
		std::vector<std::string> dirs;
		dirs = Rain::getDirs(directory, "*");

		std::vector<std::string> relpath;
		for (size_t a = 2; a < dirs.size(); a++) {
			relpath.push_back(dirs[a] + "\\");

			std::vector<std::string> subrel;
			subrel = getDiresRec(directory + dirs[a] + "\\", format);

			for (size_t b = 0; b < subrel.size(); b++)
				relpath.push_back(dirs[a] + "\\" + subrel[b]);
		}
		return relpath;
	}

	void printToFile(std::string filename, std::string *output, bool append) {
		std::ofstream out;
		if (append)
			out.open(filename, std::ios_base::out | std::ios_base::binary | std::ios_base::app);
		else
			out.open(filename, std::ios_base::out | std::ios_base::binary);
		out << *output;
		out.close();
	}
	void printToFile(std::string filename, std::string output, bool append) {
		printToFile(filename, &output, append);
	}

	std::size_t getFileSize(std::string file) {
		std::ifstream t(file, std::ios::binary);
		t.seekg(0, std::ios::end);
		std::size_t size = static_cast<std::size_t>(t.tellg());
		t.close();
		return size;
	}

	void readFileToStr(std::string filePath, std::string &fileData) {
		std::ifstream t(filePath, std::ios::binary);
		t.seekg(0, std::ios::end);
		size_t size = static_cast<size_t>(t.tellg());
		fileData = std::string(size, ' ');
		t.seekg(0);
		t.read(&fileData[0], size);
		t.close();
	}

	std::vector<std::string> readMultilineFile(std::string filePath) {
		static std::ifstream fileIn;
		std::vector<std::string> ret;

		fileIn.open(filePath, std::ios::binary);
		std::stringstream ss;
		ss << fileIn.rdbuf();

		static std::string value = "";
		std::getline(ss, value);

		while (value.length() != 0) {
			ret.push_back(Rain::strTrimWhite(value));
			value = "";
			std::getline(ss, value);
		}

		fileIn.close();
		return ret;
	}

	std::map<std::string, std::string> readParameterStream(std::stringstream &paramStream) {
		static std::string key = "", value;
		std::map<std::string, std::string> params;
		std::getline(paramStream, key, ':');

		while (key.length() != 0) {
			std::getline(paramStream, value);
			Rain::strTrimWhite(value);
			Rain::strTrimWhite(key);
			params[key] = value;
			key = "";
			std::getline(paramStream, key, ':');
		}
		return params;
	}
	std::map<std::string, std::string> readParameterString(std::string paramString) {
		std::stringstream ss;
		ss << paramString;
		return readParameterStream(ss);
	}
	std::map<std::string, std::string> readParameterFile(std::string filePath) {
		static std::ifstream fileIn;

		fileIn.open(filePath, std::ios::binary);
		std::stringstream ss;
		ss << fileIn.rdbuf();
		std::map<std::string, std::string> ret = readParameterStream(ss);
		fileIn.close();
		return ret;
	}
}