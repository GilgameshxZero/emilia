/*
Standard

All utilities dealing with files.

When in doubt: directories are specified with an ending '/'. Sometimes functions may not work if this format is not followed.
Assume functions don't work with unicode/UTF8 multibyte strings, unless specified.

Directory: string ending in / which points to a directory
File: just the filename of a file
Path: concatenated directory and filename to a file

Windows-specific.
*/

#pragma once

#include "utility-string.h"
#include "windows-lam-include.h"

#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace Rain {
	//turn a string into LPCWSTR with a "\\?\" prefix, for use in long paths unicode functions
	std::wstring pathToLongPath(std::string path);
	std::wstring pathToLongPath(std::wstring path);

	//test if file exists; either relative or absolute path work
	bool fileExists(std::string file);
	bool dirExists(std::string dir);

	//test if a path is under another path
	bool isSubPath(std::string parentPath, std::string childPath);

	//append \\ on the end if doesn't have
	std::string *standardizeDirPath(std::string *dir);
	std::string standardizeDirPath(std::string dir);

	//returns current directory of the executable (not the same as the path to the executable, sometimes)
	std::string getWorkingDirectory();

	//works with Unicode/UTF8-multibyte
	std::string getExePath();

	//converts a path to a file into either the filename or the directory for that file 
	std::string getPathDir(std::string path);
	std::string getPathFile(std::string path);

	//Get all files or all directories in a directory, in a certain format. NOTE: Takes and returns UTF8 multibyte strings - but works with unicode directories.
	std::vector<std::string> getFiles(std::string directory, std::string format = "*");
	std::vector<std::string> getDirs(std::string directory, std::string format = "*");

	//gets all files and directories under a directory, recursively
	//works with unicode/multibyte UTF8
	//use absolute paths in ignore
	//can specify only directories or files that we want; overwritten by ignore
	std::vector<std::string> getFilesRec(std::string directory, std::string format = "*", std::set<std::string> *ignore = NULL, std::set<std::string> *want = NULL);
	std::vector<std::string> getDirsRec(std::string directory, std::string format = "*", std::set<std::string> *ignore = NULL, std::set<std::string> *want = NULL);

	//creates parent directories until specified directory created
	void createDirRec(std::string dir);

	//removes all directories and files under a directory, but not the directory itself
	//works with unicode/multibyte UTF8
	//second argument specifies files/directories to not remove; use \ end to specify dir
	//use absolute paths in ignore
	void rmDirRec(std::string dir, std::set<std::string> *ignore = NULL, std::set<std::string> *want = NULL);

	//copies directory structure over, replacing any files in the destination, but not deleting any unrelated files
	void cpyDirRec(std::string src, std::string dst, std::set<std::string> *ignore = NULL);

	//converts any path to an absolute path
	std::string pathToAbsolute(std::string path);

	//allows for retrieval of file information and testing if two paths point to the same file
	BY_HANDLE_FILE_INFORMATION getFileInformation(std::string path);
	bool isEquivalentPath(std::string path1, std::string path2);

	//get last modified time of files
	//works with unicode/multibyte UTF8
	FILETIME getLastModTime(std::string path);

	//output information to a file, shorthand
	void printToFile(std::string filename, std::string *output, bool append = false);
	void printToFile(std::string filename, std::string output, bool append = false);

	std::size_t getFileSize(std::string file);
	time_t getFileLastModifyTime(std::string file);

	//put the whole file into a string
	//returns nothing, because copy constructor might be expensive
	void readFileToStr(std::string filePath, std::string &fileData);

	bool isFileWritable(std::string file);
	bool isDirEmpty(std::string dir);

	//get temporary filename in string
	std::string getTmpFileName();
}