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

const std::string DEFAULT_DIR = "C:\\Main\\Active\\Documents\\Programming\\Rain\\Developing\\Monochrome0x\\3. HTTPServerFull\\",
	DEL_EXE_PATH = "Emilia-tan%20-%20DeleteDirorFile%20(Release%20x64).exe",
	POST_EXE_PATH = "Emilia-tan%20-%20POSTToFile%20(Release%20x64).exe";
const bool USE_EXE_AS_DEFAULT = true;

void GetFiles (std::string directory, std::vector<std::string> &rtrn, std::string format);
void GetDirectories (std::string directory, std::vector<std::string> &rtrn, std::string format);
std::string GetExePath ();
std::string GetShortName (std::string directory);

std::string DecodeURLPath (std::string urlpath);
std::string EncodeURLPath (std::string path);

int main ();