/*
Standard
*/

/*
All utilities dealing with files.

When in doubt: directories are specified with an ending '/'. Sometimes functions may not work if this format is not followed.
Assume functions don't work with unicode/UTF8 multibyte strings, unless specified.

Directory: string ending in / which points to a directory
File: just the filename of a file
Path: concatenated directory and filename to a file

Windows-specific.
*/

#pragma once

#include "WindowsLAMInclude.h"
#include "UtilityString.h"

#include <cstdlib>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <map>
#include <set>
#include <sstream>

namespace Rain {
	//test if file exists; either relative or absolute path work
	bool fileExists(std::string file);

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

	//creates parent directories until specified directory created
	void createDirRec(std::string dir);

	//removes all directories and files under a directory, but not the directory itself
	//works with unicode/multibyte UTF8
	void recursiveRmDir(std::string dir);

	//converts any path to an absolute path
	std::string pathToAbsolute(std::string path);

	//allows for retrieval of file information and testing if two paths point to the same file
	BY_HANDLE_FILE_INFORMATION getFileInformation(std::string path);
	bool isEquivalentPath(std::string path1, std::string path2);

	//get last modified time of files
	//works with unicode/multibyte UTF8
	FILETIME getLastModTime(std::string path);

	//gets all files and directories under a directory, recursively
	//works with unicode/multibyte UTF8
	std::vector<std::string> getFilesRec(std::string directory, std::string format = "*");
	std::vector<std::string> getDirsRec(std::string directory, std::string format = "*");

	//output information to a file, shorthand
	void printToFile(std::string filename, std::string *output, bool append = false);
	void printToFile(std::string filename, std::string output, bool append = false);

	//file size in bytes
	std::size_t getFileSize(std::string file);

	//put the whole file into a string
	//returns nothing, because copy constructor might be expensive
	void readFileToStr(std::string filePath, std::string &fileData);

	//multiple lines, each into a separate string in the vector.
	std::vector<std::string> readMultilineFile(std::string filePath);

	//read parameters from standard parameter string, organized in lines (terminated by \n) of key:value, possibly with whitespace inbetween elements
	std::map<std::string, std::string> readParameterStream(std::stringstream &paramStream);
	std::map<std::string, std::string> readParameterString(std::string paramString);
	std::map<std::string, std::string> readParameterFile(std::string filePath);
}