/*
Standard
*/

#pragma once

#include "FastFileIO.h"
#include "RainError.h"
#include "WindowsLAMInclude.h"

#include <cstdlib>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <sys/stat.h>

namespace Rain
{
	//trim possible /r at end of string
	void TrimBSR (std::string &s);

	void GetFiles (std::string directory, std::vector<std::string> &rtrn, std::string format);
	void GetDirectories (std::string directory, std::vector<std::string> &rtrn, std::string format);
	std::string GetExePath ();
	std::string GetShortName (std::string directory);

	void GetRelFilePathRec (std::string directory, std::vector<std::string> &relpath, std::string format = "*");
	void GetRelDirPathRec (std::string directory, std::vector<std::string> &relpath, std::string format = "*");
	void GetLastModTime (std::vector<std::string> &files, std::vector<FILETIME> &lastmod);
	void RecursiveRmDir (std::string path);
	bool FileExists (std::string file);

	std::string IntToStr (int x);
	std::string LLToStr (long long x);
	int StrToInt (std::string s);
	long long StrToLL (std::string s);

	char IntToBase64 (int x);
	int Base64ToInt (char c);
	void EncodeBase64 (const std::string &str, std::string &rtrn);
	void DecodeBase64 (const std::string &str, std::string &rtrn);

	std::string EncodeURL (const std::string &value);
	std::string DecodeURL (const std::string &value);

	//length of x, interpreted as a base10 string
	int IntLogLen (int x);

	HANDLE SimpleCreateThread (LPTHREAD_START_ROUTINE threadfunc, LPVOID threadparam);
}