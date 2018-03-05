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

std::string DecodeURLPath (std::string urlpath)
{
	std::string ret = "";

	for (int a = 0;a < urlpath.length ();a += 2)
		ret.push_back ((char)(-128 + urlpath[a + 1] - 'A' + 16 * (urlpath[a] - 'A')));

	return ret;
}

std::string EncodeURLPath (std::string path)
{
	std::string ret = "";

	for (int a = 0; a < path.length (); a++)
	{
		ret.push_back ((char)((((int)path[a] + 128) >> 4) + 'A'));
		ret.push_back ((char)((((int)path[a] + 128) % 16) + 'A'));
	}

	return ret;
}

int main ()
{
	std::vector<std::string> ldirs, lfiles;
	std::ifstream in;
	std::string dir, pardir, longexename, exename, filename, fileext, contenttype, delexename;
	TCHAR exenamebuf[MAX_PATH];

	//get executable filename
	GetModuleFileName (NULL, exenamebuf, sizeof (exenamebuf) / sizeof (*exenamebuf));
	longexename = exenamebuf;
	exename = GetShortName (longexename);
	//std::cout << longexename << "<br>" << exename;

	//parameter is in the format of a path, except encoded. encoding expresses everything in hexadecimal (starting with A, though) for easy decoding.
	char *param = getenv ("QUERY_STRING");

	if (USE_EXE_AS_DEFAULT)
		dir = GetExePath () + "\\";
	else
		dir = DEFAULT_DIR;

	if (param != NULL && strlen (param) > 0)
		dir = DecodeURLPath ((std::string)param);

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
	//added delete links with each link
	if (dir.back () == '\\') //directory
	{
		GetDirectories (dir, ldirs, "*");
		GetFiles (dir, lfiles, "*");
	
		//generate links, each with a different parameter to call the same script
		std::cout << "Content-type:text/html" << std::endl << std::endl
			<< "<html>"
			<< "<head>"
			<< "<title>Emilia-tan!: File Explorer</title>"
			<< "</head>"
			<< "<body>";

		std::cout << "<a href=\"/\">Back to Emilia-tan!</a><br><br>";
		std::cout << "Current directory: " << dir << "<br>";
		std::cout << "<a href=\"../" << exename << "?" << EncodeURLPath (pardir) << "\">Parent Directory</a><br>"
			<< "<br>Directories<br>";

		for (int a = 2;a < ldirs.size ();a++)
			std::cout << "<a href=\"../" << DEL_EXE_PATH << "?" << EncodeURLPath (dir + ldirs[a] + '\\') << "\">Delete</a> "
				<< "<a href=\"../" << exename << "?" << EncodeURLPath (dir + ldirs[a] + '\\') << "\">" << ldirs[a] << "</a><br>";

		std::cout << "<br>Files<br>";
		for (int a = 0;a < lfiles.size ();a++)
			std::cout << "<a href=\"../" << DEL_EXE_PATH << "?" << EncodeURLPath (dir + lfiles[a]) << "\">Delete</a> "
				<< "<a href=\"../" << exename << "?" << EncodeURLPath (dir + lfiles[a]) << "\">" << lfiles[a] << "</a><br>";

		//add a form to insert file into this directory
		std::cout << "<br><form action = \"../" << POST_EXE_PATH << "?" << EncodeURLPath (dir) << "\" method = \"post\" enctype = \"multipart/form-data\">"
			<< "Upload File To Directory:<input type = \"file\" name = \"file\" id = \"file\">"
			<< "<input type = \"submit\" value = \"Upload\" name = \"Upload\">"
			<< "</form>";

		//add a form to add a directory
		//std::cout << "<br>

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