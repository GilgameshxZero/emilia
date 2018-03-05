/*
Given a directory (from the URL), display links to access all files and subdirectories.
Include a parent directory link.
To switch directories, update URL.
If parameter is invalid, go to some default directory.
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

std::string DecodeURLPath (std::string urlpath);
bool IsFile (std::string path);
void RecursiveRmDir (std::string path);

int main ();