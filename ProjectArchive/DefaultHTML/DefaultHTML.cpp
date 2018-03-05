#define _CRT_SECURE_NO_WARNINGS

#include "DefaultHTML.h"

int main ()
{
	std::cout << "Content-type:text/html" << std::endl << std::endl
		<< "<html>"
		<< "<head>"
		<< "<title>"
		<< "Emilia-tan!"
		<< "</title>"
		<< "<link rel=\"shortcut icon\" href=\"favicon.ico\" />"
		<< "</head>"
		<< "<body>"
		<< "<img src=\"emilia2.gif\" alt=\"E.M.T!\"><br />"
		<< "<a href=\"http://waifu.emilia-tan.com\">Waifu</a><br />";

	//check URL passcode
	char *urlparam = getenv ("QUERY_STRING");
	std::string urlget;

	if (urlparam != NULL && strlen (urlparam) > 0)
		urlget = urlparam;
	else
		urlget = "";

	std::string password;
	std::ifstream in (PASS_FILE, std::ios::binary);
	std::getline (in, password);
	in.close ();

	if (urlget == password) //display full page
	{
		std::cout << "<form action=\"/POSTToFile.exe\" method=\"post\" enctype=\"multipart/form-data\">"
			<< "Upload File To Server:"
			<< "<input type=\"file\" name=\"file\" id=\"file\">"
			<< "<input type=\"submit\" value=\"Upload\" name=\"Upload\">"
			<< "</form>"
			<< "Hosted by Gilgamesh.<br />"
			<< "<a href=\"/LinkListFiles.exe\">List Files on Server Root</a><br />"
			<< "<a href=\"/Emilia-tan%20-%20FileExplorer%20(Release%20x86).exe\">FileExplorer</a><br />";
	}
	else //todo: add form for password
	{
	}

	std::cout << "</body>"
		<< "</html>";
}