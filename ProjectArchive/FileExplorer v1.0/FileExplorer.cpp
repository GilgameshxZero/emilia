#define _CRT_SECURE_NO_WARNINGS

#include "FileExplorer.h"

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

std::string GetExePath ()
{
	char buffer[MAX_PATH];
	GetModuleFileName (NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string (buffer).find_last_of ("\\/");

	return std::string (buffer).substr (0, pos);
}

std::string GetShortName (std::string directory)
{
	std::string dirname;
	bool seechar = false;
	for (int a = directory.length () - 1; a >= 0; a--)
	{
		if (directory[a] == ':')
		{
			dirname = "C:\\";
			break;
		}
		else if (directory[a] == '\\' && seechar)
		{
			dirname = directory.substr (a + 1, directory.length () - a - 1);
			break;
		}
		else
			seechar = true;
	}

	return dirname;
}

int main ()
{
	std::vector<std::string> ldirs, lfiles;
	std::ifstream in;
	std::ofstream out;
	std::string clinkstr, dir, pardir, longexename, exename, filename, fileext, contenttype;
	TCHAR exenamebuf[MAX_PATH];
	int clink;

	//get executable filename
	GetModuleFileName (NULL, exenamebuf, sizeof (exenamebuf) / sizeof (*exenamebuf));
	longexename = exenamebuf;
	exename = GetShortName (longexename);
	//std::cout << longexename << "<br>" << exename;

	//parameter is in the format "link=INTEGER"
	char *param = getenv ("QUERY_STRING");
	int linknum = -1;

	//std::cout << param << "<br>" << linknum;
	if (param != NULL && strlen (param) >= 5)
		linknum = atoi (param + 5);
	//std::cout << param << "<br>" << linknum;

	in.open (INPUT_FILE);
	std::getline (in, clinkstr);

	//if no parameter is passed, or the input file is empty
	if (!in.good () || linknum == -1)
	{
		if (USE_EXE_AS_DEFAULT)
			dir = GetExePath ();
		else
			dir = DEFAULT_DIR;
	}
	else
	{
		clink = atoi (clinkstr.c_str ());
		for (int a = 0;a <= linknum;a++)
			std::getline (in, dir);
	}

	in.close ();

	pardir = dir;
	bool seechar = false;
	for (int a = pardir.length () - 1;a >= 0;a--)
	{
		if (pardir[a] == ':')
			break;
		else if (pardir[a] == '\\' && seechar)
		{
			pardir = pardir.substr (0, a + 1);
			break;
		}
		else
			seechar = true;
	}

	//if dir points to a file, serve that file to the browser. if not, display subdirectories and files
	if (dir.back () == '\\') //directory
	{
		GetDirectories (dir, ldirs, "*");
		GetFiles (dir, lfiles, "*");

		//output link directories back to the INPUT_FILE
		out.open (INPUT_FILE);
		out << ldirs.size () - 1 << '\n'
			<< pardir << '\n';
		for (int a = 2;a < ldirs.size ();a++) //start at 2 to ignore . and ..
			out << dir << ldirs[a] << "\\\n";
		for (int a = 0;a < lfiles.size ();a++)
			out << dir << lfiles[a] << '\n';
		out.close ();
	
		//generate links, each with a different parameter to call the same script
		std::cout << "Content-type:text/html" << std::endl << std::endl
			<< "<html>"
			<< "<head>"
			<< "<title>File List (non-recursive)</title>"
			<< "</head>"
			<< "<body>";

		std::cout << "<a href=\"/\">Back to Emilia-tan!</a><br><br>";
		std::cout << "Current directory: " << dir << "<br>";
		std::cout << "<a href=\"../" << exename << "?link=0\">Parent Directory</a><br>"
			<< "<br>Directories<br>";

		for (int a = 2;a < ldirs.size ();a++)
			std::cout << "<a href=\"../" << exename << "?link=" << a - 1 << "\">" << ldirs[a] << "</a><br>";

		std::cout << "<br>Files<br>";
		for (int a = 0;a < lfiles.size ();a++)
			std::cout << "<a href=\"../" << exename << "?link=" << ldirs.size () + a - 1 << "\">" << lfiles[a] << "</a><br>";

		std::cout << "</body>"
			<< "</html>";
	}
	else //file, serve it to browser
	{
		filename = GetShortName (dir);
		for (int a = filename.length () - 1;a >= 0;a--)
		{
			if (filename[a] == '.')
			{
				fileext = filename.substr (a + 1, filename.length () - a - 1);
				break;
			}
		}

		//try to get the right contenttype
		contenttype = "application/octet-stream";
		if (fileext == "pdf")
			contenttype = "application/pdf";
		if (fileext == "aac")
			contenttype = "audio/x-aac";
		if (fileext == "avi")
			contenttype = "video/x-msvideo";
		if (fileext == "bmp")
			contenttype = "image/bmp";
		if (fileext == "torrent")
			contenttype = "application/x-bittorrent";
		if (fileext == "c")
			contenttype = "text/x-c";
		if (fileext == "csv")
			contenttype = "text/csv";
		if (fileext == "gif")
			contenttype = "image/gif";
		if (fileext == "html")
			contenttype = "text/html";
		if (fileext == "ico")
			contenttype = "image/x-icon";
		if (fileext == "java")
			contenttype = "text/x-java-source,java";
		if (fileext == "jpeg")
			contenttype = "image/jpeg";
		if (fileext == "jpg")
			contenttype = "image/jpeg";
		if (fileext == "docx")
			contenttype = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
		if (fileext == "ppt")
			contenttype = "application/vnd.ms-powerpoint";
		if (fileext == "pub")
			contenttype = "application/x-mspublisher";
		if (fileext == "wma")
			contenttype = "audio/x-ms-wma";
		if (fileext == "doc")
			contenttype = "application/msword";
		if (fileext == "mid")
			contenttype = "audio/midi";
		if (fileext == "mpeg")
			contenttype = "video/mpeg";
		if (fileext == "mp4a")
			contenttype = "audio/mp4";
		if (fileext == "mp4")
			contenttype = "video/mp4";
		if (fileext == "png")
			contenttype = "image/png";
		if (fileext == "webm")
			contenttype = "video/webm";
		if (fileext == "tiff")
			contenttype = "image/tiff";
		if (fileext == "txt")
			contenttype = "text/plain";
		if (fileext == "wav")
			contenttype = "audio/x-wav";
		if (fileext == "zip")
			contenttype = "application/zip";

		if (fileext == "mp3")
			contenttype = "audio/mpeg";
		if (fileext == "flac")
			contenttype = "audio/flac";
		if (fileext == "ogg")
			contenttype = "audio/ogg";

		//try to open in browser, and download if not possible
		std::cout << "Content-type:" << contenttype << std::endl
			<< "Content-disposition: inline; filename=" << filename << std::endl << std::endl;
		
		in.open (dir, std::ios::binary);
		_setmode (_fileno (stdout), _O_BINARY); //set output as binary
		std::cout << in.rdbuf ();
	}

	return 0;
}