/*
Standard
*/

/*
All utilities dealing with files.
*/

#pragma once

#include "RainWindowsLAM.h"
#include "RainUtilityString.h"

#include <cstdlib>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <map>
#include <sstream>

namespace Rain {
	bool fileExists(std::string file);

	//Get all files in a directory, in a certain format. NOTE: Takes and returns UTF8 multibyte strings - but works with unicode directories.
	void getFiles(std::string directory, std::vector<std::string> &rtrn, std::string format);

	//Get all subdirectories in a directory, in a certain format. NOTE: Takes and returns UTF8 multibyte strings - but works with unicode directories.
	void getDirectories(std::string directory, std::vector<std::string> &rtrn, std::string format);

	//returns UTF8
	std::string getExePath();

	//works with both Unicode and UTF8 (untested)
	std::string getExePath(std::string directory);

	//takes and returns UTF8 strings, but uses unicode
	void getRelFilePathRec(std::string directory, std::vector<std::string> &relpath, std::string format = "*");

	//takes and returns UTF8 strings, but uses unicode
	void getRelDirPathRec(std::string directory, std::vector<std::string> &relpath, std::string format = "*");

	//takes and returns UTF8 strings, but uses unicode
	void getLastModTime(std::vector<std::string> &files, std::vector<FILETIME> &lastmod);

	//takes and returns UTF8 strings, but uses unicode
	void recursiveRmDir(std::string path);

	//shorthand function to output things to a file
	void fastOutputFileRef(std::string filename, std::string &output, bool append = false);
	void fastOutputFile(std::string filename, std::string output, bool append = false);

	//read parameters from standard parameter file, organized in lines (terminated by \n) of key:value, possibly with whitespace inbetween elements
	std::map<std::string, std::string> &readParameterFile(std::string filePath, std::map<std::string, std::string> &params);
	std::map<std::string, std::string> &readParameterString(std::string paramString, std::map<std::string, std::string> &params);
	std::map<std::string, std::string> &readParameterStream(std::stringstream &paramStream, std::map<std::string, std::string> &params);

	//returns current directory of the executable (not the same as the path to the executable, sometimes)
	std::string getWorkingDirectory();

	std::string &readFullFile(std::string filePath, std::string &fileData);

	std::string getPathDirectory(std::string path);
}