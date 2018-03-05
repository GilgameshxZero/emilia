#define _CRT_SECURE_NO_WARNINGS

#include "POSTToFile.h"

using namespace std;

ll GetHash (string &s)
{
	ll rtrn = 0;
	for (int a = 0; a < s.length (); a++)
		rtrn = ((rtrn * HMUL) + (s[a] + 128)) % HMOD;
	return rtrn;
}

ll FastModPow (ll base, ll mod, ll pow)
{
	if (pow == 0)
		return 1;
	else if (pow == 1)
		return base;
	else if ((pow & 1) == 0)
	{
		ll half = FastModPow (base, mod, pow >> 1);
		return (half * half) % mod;
	}
	else
	{
		ll half = FastModPow (base, mod, pow >> 1);
		return (((half * half) % mod) * base) % mod;
	}
}

std::string DecodeURLPath (std::string urlpath)
{
	std::string ret = "";

	for (int a = 0; a < urlpath.length (); a += 2)
		ret.push_back ((char)(-128 + urlpath[a + 1] - 'A' + 16 * (urlpath[a] - 'A')));

	return ret;
}

int main ()
{
	int temp = _setmode (_fileno (stdout), _O_BINARY);
	cout << "Content-type:text/html\r\n\r\n";
	cout << "<html>\n";
	cout << "<head>\n";
	cout << "<title>POSTToFile Success!</title>\n";
	cout << "</head>\n";
	cout << "<body>\n";
	cout << "Successfully generated file output!";

	//open stdin as binary
	temp = _setmode (_fileno (stdin), _O_BINARY);
	//cout << "<br>temp = " << temp;

	char *clenstr = NULL;
	size_t strsz = 0;
	ll clen;
	_dupenv_s (&clenstr, &strsz, "CONTENT_LENGTH");
	clen = atoi (clenstr);

	char *buffer = NULL;
	string postdata;
	buffer = new char[clen];
	fread (buffer, sizeof (char), clen, stdin);
	for (int a = 0; a < clen; a++)
		postdata.push_back (buffer[a]);
		//cout << (int)(buffer[a]) << ' ';

	//get boundary deliminator, read until /n
	string bdelim;
	for (int a = 0; a < postdata.length (); a++)
	{
		if (postdata[a] == '\n')
		{
			bdelim = postdata.substr (0, a);
			break;
		}
	}

	//use rabin-karp to find filename tag, to find double newline, and to find the next boundary deliminator
	//double newline works for both windows and unix systems
	//care - when bdelim is shorter than filename, no error handling
	string fntag = "filename", filename = "", doublenl = "\n\n", windowsnl = "\r\n\r\n";
	ll fnthash = GetHash (fntag), curfnthash = GetHash (postdata.substr (bdelim.length () - fntag.length (), fntag.length ())), fntmp = FastModPow (HMUL, HMOD, fntag.length () - 1);
	ll dnlhash = GetHash (doublenl), curdnlhash = GetHash (postdata.substr (bdelim.length () - doublenl.length (), doublenl.length ())), dnlmp = FastModPow (HMUL, HMOD, doublenl.length () - 1);
	ll wnlhash = GetHash (windowsnl), curwnlhash = GetHash (postdata.substr (bdelim.length () - windowsnl.length (), windowsnl.length ())), wnlmp = FastModPow (HMUL, HMOD, windowsnl.length () - 1);
	ll bdmhash = GetHash (bdelim), curbdmhash = GetHash (postdata.substr (bdelim.length () - bdelim.length (), bdelim.length ())), bdmmp = FastModPow (HMUL, HMOD, bdelim.length () - 1);
	ll databegin = -1, dataend = -1;
	bool winlineend = false;
	//cout << "<br>bdmhash: " << bdmhash;
	//cout << "<br>curbdmhash: " << curbdmhash;
	for (int a = bdelim.length (); a < postdata.length (); a++)
	{
		curfnthash = ((((curfnthash - (fntmp * (postdata[a - fntag.length ()] + 128)) % HMOD) % HMOD + HMOD) % HMOD) * HMUL + postdata[a] + 128) % HMOD;
		curdnlhash = ((((curdnlhash - (dnlmp * (postdata[a - doublenl.length ()] + 128)) % HMOD) % HMOD + HMOD) % HMOD) * HMUL + postdata[a] + 128) % HMOD;
		curwnlhash = ((((curwnlhash - (wnlmp * (postdata[a - windowsnl.length ()] + 128)) % HMOD) % HMOD + HMOD) % HMOD) * HMUL + postdata[a] + 128) % HMOD;
		curbdmhash = ((((curbdmhash - (bdmmp * (postdata[a - bdelim.length ()] + 128)) % HMOD) % HMOD + HMOD) % HMOD) * HMUL + postdata[a] + 128) % HMOD;
		//cout << "<br>curbdmhash: " << curbdmhash;
		//cout << "<br>sub: " << postdata[a - bdelim.length ()];
		//cout << "<br>add: " << postdata[a];

		if (curfnthash == fnthash && filename == "")
		{
			bool equal = true;
			for (int b = 0; b < fntag.length (); b++)
			{
				if (fntag[b] != postdata[a - fntag.length () + b + 1])
				{
					equal = false;
					break;
				}
			}

			if (equal)
			{
				for (int b = a + 3; b < postdata.length (); b++)
				{
					if (postdata[b] == '\"')
					{
						filename = postdata.substr (a + 3, b - a - 3);
						break;
					}
				}
			}
		}

		if (curdnlhash == dnlhash && databegin == -1)
		{
			bool equal = true;
			for (int b = 0; b < doublenl.length (); b++)
			{
				if (doublenl[b] != postdata[a - doublenl.length () + b + 1])
				{
					equal = false;
					break;
				}
			}

			if (equal)
			{
				databegin = a + 1;
				winlineend = false;
			}
		}

		if (curwnlhash == wnlhash && databegin == -1)
		{
			bool equal = true;
			for (int b = 0; b < windowsnl.length (); b++)
			{
				if (windowsnl[b] != postdata[a - windowsnl.length () + b + 1])
				{
					equal = false;
					break;
				}
			}

			if (equal)
			{
				databegin = a + 1;
				winlineend = true;
			}
		}

		if (curbdmhash == bdmhash && dataend == -1)
		{
			bool equal = true;
			for (int b = 0; b < bdelim.length (); b++)
			{
				if (bdelim[b] != postdata[a - bdelim.length () + b + 1])
				{
					equal = false;
					break;
				}
			}

			if (equal)
			{
				dataend = a + 1 - bdelim.length ();

				//ignore last newline, depending on whether its \r\n or just \n, from the double newline we read earlier
				if (winlineend)
					dataend -= 2;
				else
					dataend -= 1;
			}
		}
	}

	//ofstream out ("C:/Users/Yang Yan/Desktop/Main/Active/Emilia-tan/Root/file.txt", ios::binary);
	//out << postdata.substr (databegin, dataend - databegin);
	//out << postdata;
	//out.close ();

	//get referrer and also path param, if it exists. if not, default to a directory
	std::string filepath, refer;
	char *buf2;

	buf2 = getenv ("QUERY_STRING");

	if (buf2 == NULL || strlen (buf2) == 0)
		filepath = DEFAULT_DIR;
	filepath = DecodeURLPath (buf2);

	buf2 = getenv ("HTTP_REFERER");
	if (buf2 == NULL)
		return -2;
	refer = buf2;

	ofstream fileout (filepath + filename, ios::binary);
	fileout.write (buffer + databegin, dataend - databegin);
	fileout.close ();

	cout << "<br>Length of POST content: " << clenstr;
	cout << "<br>Length of buffer: " << clen;
	cout << "<br>Length of postdata: " << postdata.length ();
	cout << "<br>Boundary deliminator: " << bdelim;
	cout << "<br>Filename: " << filename;
	cout << "<br>Beginning of data: " << databegin;
	cout << "<br>End of data: " << dataend;
	cout << "<br>Length of filedata: " << dataend - databegin;
	
	cout << "<br><a href=\"" << refer << "\">Return (to Emilia-tan)!</a><br>";

	cout << "</body>\n";
	cout << "</html>\n";

	delete[] clenstr;
	delete[] buffer;

	return 0;
}