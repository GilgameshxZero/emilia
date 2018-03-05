#include "Utility.h"

namespace Rain
{
	void TrimBSR (std::string &s)
	{
		if (!s.empty () && s.back () == '\r')
			s.pop_back ();
	}

	//Get all files in a directory, in a certain format. NOTE: Takes and returns UTF8 multibyte strings - but works with unicode directories.
	void GetFiles (std::string directory, std::vector<std::string> &rtrn, std::string format)
	{
		char search_path[MAX_PATH], multibyte[MAX_PATH];
		wchar_t unicodesp[MAX_PATH];
		sprintf_s (search_path, ("%s/" + format).c_str (), directory.c_str ());
		WIN32_FIND_DATAW fd;

		MultiByteToWideChar (CP_UTF8, 0, search_path, -1, unicodesp, MAX_PATH);
		int k = GetLastError ();
		HANDLE hFind = ::FindFirstFileW (unicodesp, &fd);

		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					WideCharToMultiByte (CP_UTF8, 0, fd.cFileName, -1, multibyte, MAX_PATH, NULL, NULL);
					rtrn.push_back (multibyte);
				}
			} while (::FindNextFileW (hFind, &fd));
			::FindClose (hFind);
		}
	}

	//Get all subdirectories in a directory, in a certain format. NOTE: Takes and returns UTF8 multibyte strings - but works with unicode directories.
	void GetDirectories (std::string directory, std::vector<std::string> &rtrn, std::string format)
	{
		char search_path[MAX_PATH], multibyte[MAX_PATH];
		wchar_t unicodesp[MAX_PATH];
		sprintf_s (search_path, ("%s/" + format).c_str (), directory.c_str ());
		WIN32_FIND_DATAW fd;

		MultiByteToWideChar (CP_UTF8, 0, search_path, -1, unicodesp, MAX_PATH);
		int k = GetLastError ();
		HANDLE hFind = ::FindFirstFileW (unicodesp, &fd);

		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					WideCharToMultiByte (CP_UTF8, 0, fd.cFileName, -1, multibyte, MAX_PATH, NULL, NULL);
					rtrn.push_back (multibyte);
				}
			} while (::FindNextFileW (hFind, &fd));
			::FindClose (hFind);
		}
	}

	//returns UTF8
	std::string GetExePath ()
	{
		char multibyte[MAX_PATH];
		wchar_t buffer[MAX_PATH];

		GetModuleFileNameW (NULL, buffer, MAX_PATH);
		WideCharToMultiByte (CP_UTF8, 0, buffer, -1, multibyte, MAX_PATH, NULL, NULL);

		std::string::size_type pos = std::string (multibyte).find_last_of ("\\/");

		return std::string (multibyte).substr (0, pos);
	}

	//takes and returns UTF8 strings, but uses unicode
	void GetRelFilePathRec (std::string directory, std::vector<std::string> &relpath, std::string format)
	{
		std::vector<std::string> dirs, files;
		Rain::GetDirectories (directory, dirs, "*");
		Rain::GetFiles (directory, files, format);

		for (std::size_t a = 0; a < files.size (); a++)
			relpath.push_back (files[a]);

		for (std::size_t a = 2; a < dirs.size (); a++)
		{
			std::vector<std::string> subrel;
			GetRelFilePathRec (directory + dirs[a] + "\\", subrel, format);

			for (std::size_t b = 0; b < subrel.size (); b++)
				relpath.push_back (dirs[a] + "\\" + subrel[b]);
		}
	}

	//takes and returns UTF8 strings, but uses unicode
	void GetRelDirPathRec (std::string directory, std::vector<std::string> &relpath, std::string format)
	{
		std::vector<std::string> dirs;
		Rain::GetDirectories (directory, dirs, "*");

		for (size_t a = 2; a < dirs.size (); a++)
		{
			relpath.push_back (dirs[a] + "\\");

			std::vector<std::string> subrel;
			GetRelDirPathRec (directory + dirs[a] + "\\", subrel, format);

			for (size_t b = 0; b < subrel.size (); b++)
				relpath.push_back (dirs[a] + "\\" + subrel[b]);
		}
	}

	//takes and returns UTF8 strings, but uses unicode
	void GetLastModTime (std::vector<std::string> &files, std::vector<FILETIME> &lastmod)
	{
		wchar_t unicode[MAX_PATH];

		for (size_t b = 0; b < files.size (); b++)
		{
			MultiByteToWideChar (CP_UTF8, 0, files[b].c_str (), -1, unicode, MAX_PATH);
			HANDLE hfile = CreateFileW (unicode, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			lastmod.push_back (FILETIME ());

			if (hfile == INVALID_HANDLE_VALUE)
				lastmod.back ().dwLowDateTime = lastmod.back ().dwHighDateTime = 0;
			else
			{
				GetFileTime (hfile, NULL, NULL, &lastmod.back ());
				CloseHandle (hfile);
			}
		}
	}

	//takes and returns UTF8 strings, but uses unicode
	void RecursiveRmDir (std::string path)
	{
		wchar_t unicode[MAX_PATH];
		std::vector<std::string> ldir, lfile;

		GetDirectories (path, ldir, "*");
		GetFiles (path, lfile, "*");

		for (std::size_t a = 0; a < lfile.size (); a++)
		{
			MultiByteToWideChar (CP_UTF8, 0, (path + lfile[a]).c_str (), -1, unicode, MAX_PATH);
			DeleteFileW (unicode);
		}
		for (std::size_t a = 2; a < ldir.size (); a++) //skip . and ..
		{
			MultiByteToWideChar (CP_UTF8, 0, (path + ldir[a] + '\\').c_str (), -1, unicode, MAX_PATH);
			RecursiveRmDir (path + ldir[a] + '\\');
			RemoveDirectoryW (unicode);
		}
	}

	//works with both Unicode and UTF8 (untested)
	std::string GetShortName (std::string directory)
	{
		std::string dirname;
		bool seechar = false;
		for (std::size_t a = directory.length () - 1; a >= 0; a--)
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

	bool FileExists (std::string file)
	{
		struct stat buffer;
		return (stat (file.c_str (), &buffer) == 0);
	}

	std::string IntToStr (int x)
	{
		std::stringstream ss;
		ss << x;
		return ss.str ();
	}
	std::string LLToStr (long long x)
	{
		std::stringstream ss;
		ss << x;
		return ss.str ();
	}
	int StrToInt (std::string s)
	{
		int r;
		std::stringstream ss (s);
		ss >> r;
		return r;
	}
	long long StrToLL (std::string s)
	{
		long long r;
		std::stringstream ss (s);
		ss >> r;
		return r;
	}

	char IntToBase64 (int x)
	{
		if (x < 26)
			return x + 'A';
		if (x < 52)
			return x - 26 + 'a';
		if (x < 62)
			return x - 52 + '0';
		if (x == 62)
			return '+';
		else //if (x == 63)
			return '/';
	}
	int Base64ToInt (char c)
	{
		if (c == '/')
			return 63;
		if (c == '+')
			return 62;
		if (c < 58)
			return c - '0' + 52;
		if (c < 91)
			return c - 'A';
		else //if (c < 123)
			return c - 'a' + 26;
	}
	void EncodeBase64 (const std::string &str, std::string &rtrn)
	{
		int bitsinchar = 0;
		int curchar = 0;
		static const int MAXBITS = 6;
		static const unsigned char MASK = 0xFF;
		for (std::size_t a = 0; a < str.length (); a++)
		{
			curchar |= ((str[a] & (MASK << (bitsinchar + 2))) >> (bitsinchar + 2));
			rtrn.push_back (IntToBase64 (curchar));
			curchar = (str[a] & (MASK >> (MAXBITS - bitsinchar)));
			bitsinchar = bitsinchar + 2;
			curchar <<= MAXBITS - bitsinchar;

			if (bitsinchar == MAXBITS)
			{
				rtrn.push_back (IntToBase64 (curchar));
				curchar = 0;
				bitsinchar = 0;
			}
		}

		if (bitsinchar != 0)
			rtrn.push_back (IntToBase64 (curchar));
	}
	void DecodeBase64 (const std::string &str, std::string &rtrn)
	{
		int bitsinchar = 0;
		char curchar = 0, curint;
		static const int MAXBITS = 8;
		static const unsigned char MASK = 0xFF;
		for (std::size_t a = 0; a < str.length (); a++)
		{
			curint = Base64ToInt (str[a]);
			if (bitsinchar == 0)
			{
				curchar = (curint << 2);
				bitsinchar = 6;
				continue;
			}
			curchar |= (((curint << 2) & (MASK << bitsinchar)) >> bitsinchar);
			rtrn.push_back (curchar);
			curchar = (curint & (MASK >> (10 - bitsinchar)));
			bitsinchar = bitsinchar - 2;
			curchar <<= MAXBITS - bitsinchar;
		}
	}

	//EncodeURL taken from http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
	std::string EncodeURL (const std::string &value)
	{
		std::ostringstream escaped;
		escaped.fill ('0');
		escaped << std::hex;

		for (std::string::const_iterator i = value.begin (), n = value.end (); i != n; ++i) {
			std::string::value_type c = (*i);

			// Keep alphanumeric and other accepted characters intact
			if (isalnum (c) || c == '-' || c == '_' || c == '.' || c == '~') {
				escaped << c;
				continue;
			}

			// Any other characters are percent-encoded
			escaped << std::uppercase;
			escaped << '%' << std::setw (2) << int ((unsigned char)c);
			escaped << std::nouppercase;
		}

		return escaped.str ();
	}
	//DecodeURL taken from http://stackoverflow.com/questions/2673207/c-c-url-decode-library
	std::string DecodeURL (const std::string &value)
	{
		std::string rtrn;
		char a, b;
		const char *src = value.c_str ();
		while (*src) {
			if ((*src == '%') &&
				((a = src[1]) && (b = src[2])) &&
				(isxdigit (a) && isxdigit (b))) {
				if (a >= 'a')
					a -= 'a' - 'A';
				if (a >= 'A')
					a -= ('A' - 10);
				else
					a -= '0';
				if (b >= 'a')
					b -= 'a' - 'A';
				if (b >= 'A')
					b -= ('A' - 10);
				else
					b -= '0';
				rtrn += 16 * a + b;
				src += 3;
			}
			else if (*src == '+') {
				rtrn += ' ';
				src++;
			}
			else {
				rtrn += *src++;
			}
		}

		return rtrn;
	}

	int IntLogLen (int x)
	{
		int ilen = 1;
		for (int a = 10; a <= x; a *= 10)
			ilen++;
		return ilen;
	}

	HANDLE SimpleCreateThread (LPTHREAD_START_ROUTINE threadfunc, LPVOID threadparam)
	{
		return CreateThread (NULL, 0, threadfunc, threadparam, 0, NULL);
	}
}