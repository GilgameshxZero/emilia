#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <vector>

int main ()
{
	char *referrer = getenv ("HTTP_REFERER");
	std::cout << "Content-type:text/html" << std::endl << std::endl
		<< "<html>"
		<< "<head>"
		<< "<title>Emilia-tan!: Generate Small JPG</title>"
		<< "<link rel=\"shortcut icon\" href=\"/favicon.ico\" type=\"image/x-icon\" />"
		<< "</head>"
		<< "<body>";

	char *param = getenv ("QUERY_STRING");
	std::string imagecode = param;
	imagecode = imagecode.substr (10, imagecode.size () - 10); //cutoff the "imagecode="

	char *host = getenv ("HTTP_HOST");
	std::cout << "Image link: http://" << host << "/smallJPG/" << imagecode << ".jpg<br>";

	//find next image number
	std::ifstream in ("smallJPG\\" + imagecode + ".txt");
	if (in.fail ())
		std::cout << "Image has not been accessed yet.<br>";
	else
	{
		std::vector<time_t> acctime (1);

		while (in >> acctime.back ())
			acctime.push_back (0);

		acctime.pop_back ();
		std::cout << "Access times: <br>";

		for (std::size_t a = 0;a < acctime.size ();a++)
		{
			struct tm *timeinfo;
			char buffer[80];

			timeinfo = localtime (&acctime[a]);

			strftime (buffer, 80, "%Y/%m/%d %H:%M:%S", timeinfo);
			std::cout << buffer << "<br>";
		}
	}
	in.close ();

	std::cout << "<a href=\"" << referrer << "\">Back to Emilia-tan!</a><br><br>";
	std::cout << "</body>"
		<< "</html>";

	return 0;
}