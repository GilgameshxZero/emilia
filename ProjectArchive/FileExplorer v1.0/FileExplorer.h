/*
Given a directory (from FileExplorer.in in the working directory, and the URL), display links to access all files and subdirectories.
Include a parent directory link.
To switch directories, update FileExplorer.in, and reload the page, specifying which directory in the .in file to go to.
If FileExplorer.in is empty or the parameter is NULL, default to some constant directory.
*/

#pragma once

#include <io.h>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <vector>
#include <Windows.h>

const std::string DEFAULT_DIR = "C:\\Users\\Yang Yan\\Desktop\\Main\\Active\\Emilia-tan\\Root\\",
	INPUT_FILE = "FileExplorer.in";
const bool USE_EXE_AS_DEFAULT = false;

void GetFiles (std::string directory, std::vector<std::string> &rtrn, std::string format);
void GetDirectories (std::string directory, std::vector<std::string> &rtrn, std::string format);
std::string GetExePath ();
std::string GetShortName (std::string directory);

int main ();