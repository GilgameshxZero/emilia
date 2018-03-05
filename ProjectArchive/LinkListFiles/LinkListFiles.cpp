#include "LinkListFiles.h"

using namespace std;

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

std::string GetExePath ()
{
	char buffer[MAX_PATH];
	GetModuleFileName (NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string (buffer).find_last_of ("\\/");

	return std::string (buffer).substr (0, pos);
}

int main ()
{
	vector<string> files;
	string exepath = GetExePath ();

	GetFiles (exepath, files, "*");
	
	cout << "Content-type:text/html\r\n\r\n";
	cout << "<html>\n";
	cout << "<head>\n";
	cout << "<title>File List (non-recursive)</title>\n";
	cout << "</head>\n";
	cout << "<body>\n";

	for (int a = 0;a < files.size ();a++)
		cout << "<a href=\"http://www.emilia-tan.com/" << files[a] << "\">" << files[a] << "</a><br>";
	
	cout << "</body>\n";
	cout << "</html>\n";
}