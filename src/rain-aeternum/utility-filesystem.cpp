#include "utility-filesystem.h"

namespace Rain {
	std::wstring pathToLongPath(std::string path) {
		return Rain::mbStrToWStr("\\\\?\\" + path);
	}
	std::wstring pathToLongPath(std::wstring path) {
		static const std::wstring prefix = Rain::mbStrToWStr("\\\\?\\");
		return prefix + path;
	}

	bool fileExists(std::string file) {
		struct _stat buffer;
		_stat(file.c_str(), &buffer);
		return (buffer.st_mode & _S_IFREG) != 0;
	}
	bool dirExists(std::string dir) {
		DWORD ftyp = GetFileAttributesW(pathToLongPath(pathToAbsolute(dir)).c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return false;  //something is wrong with your path!
		if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
			return true;   //this is a directory!
		return false;    //this is not a directory!
	}

	bool isSubPath(std::string parentPath, std::string childPath) {
		parentPath = pathToAbsolute(parentPath);
		childPath = pathToAbsolute(childPath);
		return childPath.substr(0, parentPath.length()) == parentPath;
	}

	std::string *standardizeDirPath(std::string *dir) {
		if (dir->back() != '\\')
			dir->push_back('\\');
		return dir;
	}
	std::string standardizeDirPath(std::string dir) {
		return *standardizeDirPath(&dir);
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
		char multibyte[32767];
		wchar_t buffer[32767];

		GetModuleFileNameW(NULL, buffer, 32767);
		WideCharToMultiByte(CP_UTF8, 0, buffer, -1, multibyte, 32767, NULL, NULL);
		return std::string(multibyte);
	}

	std::string getPathDir(std::string path) {
		return path.substr(0, path.find_last_of("\\/")) + "\\";
	}
	std::string getPathFile(std::string path) {
		return path.substr(path.find_last_of("\\/") + 1, path.length());
	}

	std::vector<std::string> getFiles(std::string directory, std::string format) {
		char search_path[32767], multibyte[32767];
		wchar_t unicodesp[32767];
		sprintf_s(search_path, ("%s" + format).c_str(), directory.c_str());
		WIN32_FIND_DATAW fd;

		MultiByteToWideChar(CP_UTF8, 0, search_path, -1, unicodesp, 32767);
		HANDLE hFind = ::FindFirstFileW(pathToLongPath(std::wstring(unicodesp)).c_str(), &fd);

		std::vector<std::string> ret;
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
					WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, multibyte, 32767, NULL, NULL);
					ret.push_back(multibyte);
				}
			} while (::FindNextFileW(hFind, &fd));
			::FindClose(hFind);
		}

		return ret;
	}
	std::vector<std::string> getDirs(std::string directory, std::string format) {
		char search_path[32767], multibyte[32767];
		wchar_t unicodesp[32767];
		sprintf_s(search_path, ("%s" + format).c_str(), directory.c_str());
		WIN32_FIND_DATAW fd;

		MultiByteToWideChar(CP_UTF8, 0, search_path, -1, unicodesp, 32767);
		HANDLE hFind = ::FindFirstFileW(pathToLongPath(std::wstring(unicodesp)).c_str(), &fd);

		std::vector<std::string> ret;
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
					WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, multibyte, 32767, NULL, NULL);
					ret.push_back(multibyte);
				}
			} while (::FindNextFileW(hFind, &fd));
			::FindClose(hFind);
		}

		return ret;
	}

	std::vector<std::string> getFilesRec(std::string directory, std::string format, std::set<std::string> *ignore, std::set<std::string> *want) {
		std::vector<std::string> dirs, files;
		directory = Rain::pathToAbsolute(directory);
		dirs = Rain::getDirs(directory, "*");
		files = Rain::getFiles(directory, format);

		std::vector<std::string> relpath;
		for (std::size_t a = 0; a < files.size(); a++) {
			if (ignore == NULL || ignore->find(pathToAbsolute(directory + files[a])) == ignore->end()) {
				if (want == NULL || want->find(pathToAbsolute(directory + files[a])) != want->end()) {
					relpath.push_back(files[a]);
				}
			}
		}

		for (std::size_t a = 2; a < dirs.size(); a++) {
			if (ignore != NULL && ignore->find(pathToAbsolute(directory + dirs[a] + "\\")) != ignore->end())
				continue;

			std::vector<std::string> subrel;
			if (want == NULL || want->find(pathToAbsolute(directory + dirs[a] + "\\")) != want->end()) {
				//drop the want variable, since everything in this directory is wanted
				subrel = getFilesRec(directory + dirs[a] + "\\", format, ignore);
			}

			for (std::size_t b = 0; b < subrel.size(); b++)
				relpath.push_back(dirs[a] + "\\" + subrel[b]);
		}
		return relpath;
	}
	std::vector<std::string> getDirsRec(std::string directory, std::string format, std::set<std::string> *ignore, std::set<std::string> *want) {
		std::vector<std::string> dirs;
		directory = Rain::pathToAbsolute(directory);
		dirs = Rain::getDirs(directory, "*");

		std::vector<std::string> relpath;
		for (size_t a = 2; a < dirs.size(); a++) {
			if (ignore == NULL || ignore->find(pathToAbsolute(directory + dirs[a] + "\\")) != ignore->end())
				continue;

			std::vector<std::string> subrel;

			if (want == NULL || want->find(pathToAbsolute(directory + dirs[a] + "\\")) != want->end()) {
				relpath.push_back(dirs[a] + "\\");
				subrel = getDirsRec(directory + dirs[a] + "\\", format, ignore, want);
			}

			for (size_t b = 0; b < subrel.size(); b++)
				relpath.push_back(dirs[a] + "\\" + subrel[b]);
		}
		return relpath;
	}

	void createDirRec(std::string dir) {
		size_t pos = 0;
		dir = pathToAbsolute(dir.substr(0, dir.length() - 1)); //remove the '\\'
		do {
			pos = dir.find_first_of("\\/", pos + 1);
			CreateDirectoryW(pathToLongPath(dir.substr(0, pos)).c_str(), NULL);
		} while (pos != std::string::npos);
	}

	void rmDirRec(std::string dir, std::set<std::string> *ignore, std::set<std::string> *want) {
		wchar_t unicode[32767];
		std::vector<std::string> ldir, lfile;

		ldir = getDirs(pathToAbsolute(dir), "*");
		lfile = getFiles(pathToAbsolute(dir), "*");

		for (std::size_t a = 0; a < lfile.size(); a++) {
			MultiByteToWideChar(CP_UTF8, 0, pathToAbsolute(dir + lfile[a]).c_str(), -1, unicode, 32767);
			if (ignore == NULL || ignore->find(pathToAbsolute(dir + lfile[a])) == ignore->end()) {
				if (want == NULL || want->find(pathToAbsolute(dir + lfile[a])) != want->end()) {
					DeleteFileW(pathToLongPath(std::wstring(unicode)).c_str());
				}
			}
		}
		for (std::size_t a = 2; a < ldir.size(); a++) //skip . and ..
		{
			MultiByteToWideChar(CP_UTF8, 0, pathToAbsolute(dir + ldir[a] + '\\').c_str(), -1, unicode, 32767);
			if (ignore == NULL || ignore->find(pathToAbsolute(dir + ldir[a] + '\\')) == ignore->end()) {
				if (want == NULL || want->find(pathToAbsolute(dir + ldir[a] + '\\')) != want->end()) {
					//want everything under this directory, so don't pass any argument forward
					rmDirRec(dir + ldir[a] + '\\', ignore, NULL);

					//RemoveDirectory removes symbolic links even when not empty, so only call if the directory really is empty
					if (isDirEmpty(pathToAbsolute(dir + ldir[a] + '\\'))) {
						RemoveDirectoryW(pathToLongPath(std::wstring(unicode)).c_str());
					}
				}
			}
		}
	}

	void cpyDirRec(std::string src, std::string dst, std::set<std::string> *ignore) {
		//relative path (wrt src) of each of the files in the source directory
		src = standardizeDirPath(pathToAbsolute(src));
		dst = standardizeDirPath(pathToAbsolute(dst));
		std::vector<std::string> filesRel = getFilesRec(src, "*", ignore);

		for (std::string s : filesRel) {
			createDirRec(getPathDir(dst + s));
			CopyFileW(pathToLongPath(src + s).c_str(), pathToLongPath(dst + s).c_str(), FALSE);
		}
	}

	std::string pathToAbsolute(std::string path) {
		WCHAR tcFullPath[32767];
		GetFullPathNameW(mbStrToWStr(path).c_str(), 32767, tcFullPath, NULL);
		return wStrToMBStr(tcFullPath);
	}

	BY_HANDLE_FILE_INFORMATION getFileInformation(std::string path) {
		HANDLE handle = CreateFileW(pathToLongPath(pathToAbsolute(path)).c_str(),
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
		wchar_t unicode[32767];
		FILETIME ret;

		MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, unicode, 32767);
		HANDLE hfile = CreateFileW(unicode, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hfile == INVALID_HANDLE_VALUE)
			ret.dwLowDateTime = ret.dwHighDateTime = 0;
		else {
			GetFileTime(hfile, NULL, NULL, &ret);
			CloseHandle(hfile);
		}

		return ret;
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
		std::ifstream t(Rain::pathToAbsolute(file), std::ios::binary);
		t.seekg(0, std::ios::end);
		std::size_t size = static_cast<std::size_t>(t.tellg());
		t.close();
		return size;
	}
	time_t getFileLastModifyTime(std::string file) {
		struct _stat buffer;
		_stat(file.c_str(), &buffer);
		return buffer.st_mtime;
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

	bool isFileWritable(std::string file) {
		FILE *fp;
		fopen_s(&fp, file.c_str(), "w");
		if (fp == NULL) {
			return false;
		}
		fclose(fp);
		return true;
	}
	bool isDirEmpty(std::string dir) {
		std::vector<std::string> ldir, lfile;
		ldir = getDirs(pathToAbsolute(dir), "*");
		lfile = getFiles(pathToAbsolute(dir), "*");
		return (ldir.size() == 2 && lfile.size() == 0);
	}

	std::string getTmpFileName() {
		TCHAR path[MAX_PATH + 1];
		GetTempPath(MAX_PATH + 1, path);

		TCHAR file[MAX_PATH + 1];
		GetTempFileName(path, "", 0, file);
		return file;
	}
}