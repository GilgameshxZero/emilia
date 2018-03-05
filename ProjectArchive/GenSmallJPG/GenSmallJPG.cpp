#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <sstream>
#include <iostream>

int main ()
{
	//find next image number
	std::ifstream in ("GenSmallJPG info.txt");
	int nextnum;
	in >> nextnum;
	in.close ();

	std::ofstream out ("GenSmallJPG info.txt");
	out << nextnum + 1;
	out.close ();

	in.open ("GenSmallJPG orig.jpg", std::ios::binary);
	std::stringstream img;
	img << in.rdbuf ();
	in.close ();

	std::stringstream ss;
	ss << nextnum;
	out.open ("smallJPG\\" + ss.str () + ".jpg", std::ios::binary);
	out << img.rdbuf ();
	out.close ();

	//display page with link to go back to referrer
	char *referrer = getenv ("HTTP_REFERER"), *host = getenv ("HTTP_HOST");
	std::cout << "Content-type:text/html" << std::endl << std::endl
		<< "<html>"
		<< "<head>"
		<< "<title>Emilia-tan!: Generate Small JPG</title>"
		<< "<link rel=\"shortcut icon\" href=\"/favicon.ico\" type=\"image/x-icon\" />"
		<< "</head>"
		<< "<body>"
		<< "Link to image (insert this into your email!): http://" << host << "/smallJPG/" << nextnum << ".jpg<br>"
		<< "Image code (remember this, you will need it when you want to retrieve the first access time): " << nextnum << "<br>";

	std::cout << "<a href=\"" << referrer << "\">Back to Emilia-tan!</a><br><br>";
	std::cout << "</body>"
		<< "</html>";

	return 0;
}