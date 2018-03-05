#define _CRT_SECURE_NO_WARNINGS

#include "DeleteDirorFile.h"

//Get all files in a directory, in a certain format.
void GetFiles (std::string directory, std::vector<std::string> &rtrn, std::string format)
{
	char search_path[MAX_PATH];
	sprintf_s (search_path, ("%s/" + format).c_str (), directory.c_str ());
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile (search_path, &fd);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				rtrn.push_back (fd.cFileName);
		} while (::FindNextFile (hFind, &fd));
		::FindClose (hFind);
	}
}

//Get all subdirectories in a directory, in a certain format.
void GetDirectories (std::string directory, std::vector<std::string> &rtrn, std::string format)
{
	char search_path[MAX_PATH];
	sprintf_s (search_path, ("%s/" + format).c_str (), directory.c_str ());
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile (search_path, &fd);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				rtrn.push_back (fd.cFileName);
		} while (::FindNextFile (hFind, &fd));
		::FindClose (hFind);
	}
}

std::string DecodeURLPath (std::string urlpath)
{
	std::string ret = "";

	for (int a = 0; a < urlpath.length (); a += 2)
		ret.push_back ((char)(-128 + urlpath[a + 1] - 'A' + 16 * (urlpath[a] - 'A')));

	return ret;
}

bool IsFile (std::string path)
{
	if (path.back () == '\\')
		return false;
	return true;
}

void RecursiveRmDir (std::string path)
{
	std::vector<std::string> ldir, lfile;
	GetDirectories (path, ldir, "*");
	GetFiles (path, lfile, "*");

	for (int a = 0;a < lfile.size ();a++)
		DeleteFile ((path + lfile[a]).c_str ());
	for (int a = 2;a < ldir.size ();a++) //skip . and ..
	{
		RecursiveRmDir (path + ldir[a] + '\\');
		RemoveDirectory ((path + ldir[a] + '\\').c_str ());
	}
}

int main ()
{
	//parameter is in the format of a path, except encoded. encoding expresses everything in hexadecimal (starting with A, though) for easy decoding.
	std::string dir = "";
	char *param = getenv ("QUERY_STRING");

	if (param != NULL && strlen (param) > 0)
		dir = DecodeURLPath ((std::string)param);
	if (dir == "") //param was invalid
		return -1;

	if (IsFile (dir))
		DeleteFile (dir.c_str ());
	else
	{
		RecursiveRmDir (dir);
		RemoveDirectory (dir.c_str ());
	}

	//redirect back
	char *refer = getenv ("HTTP_REFERER");

	if (refer == NULL || strlen (refer) == 0)
		return -2;

	std::cout << "Content-type:text/html" << std::endl << std::endl
		<< "<html>"
		<< "<head>"
		<< "<meta http-equiv=\"REFRESH\" content=\"0;url=" << refer << "\">"
		<< "</head>"
		<< "<body>"
		//<< dir << "<br>" << "ret: " << delret
		<< "</body>"
		<< "</html>";

	return 0;
}