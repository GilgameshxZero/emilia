#define _CRT_SECURE_NO_WARNINGS

#include "Subscribe794.h"

int main () {
	char *cgetparam = getenv ("QUERY_STRING");
	std::string getp = cgetparam; //"email=yangawesome@gmail.com"
	//std::string getp = "email=testmail@gmail.com";

	std::cout << "Content-type:text/html" << std::endl << std::endl
		<< "<html>"
		<< "<head>"
		<< "<title>Emilia-tan!: Redirecting...</title>"
		<< "<meta http-equiv=\"refresh\" content=\"0; url = /794success.html\">"
		<< "</head>"
		<< "<body>"
		<< "Please wait, Emilia is hard at work...<br />"
		<< "</body>"
		<< "</html>";
	fclose (stdout);

	//run selenium backend async to send email
	system (("start Subscribe794PythonBackend\\Subscribe794PythonBackend.exe " + getp).c_str ());

	return 0;
}