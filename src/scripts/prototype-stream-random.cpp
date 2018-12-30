/*
Emilia-tan Script

This script continuously transfers mp3 files into cout.
*/

#include "../rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);

	srand(time(NULL));

	//parse range requests
	/*std::string rangeRaw = std::getenv("HTTP_RANGE");
	std::size_t fileBegin = 0,
		fileEnd = 0;
	if (rangeRaw != "") {
		std::string range = rangeRaw.substr(rangeRaw.find("=") + 1);
		std::size_t rangeDelim = range.find("-");
		fileBegin = Rain::strToT<std::size_t>(range.substr(0, rangeDelim));
		if (rangeDelim != range.length() - 1) {
			fileEnd = Rain::strToT<std::size_t>(range.substr(rangeDelim + 1));
		}
	}*/

	std::string response;
	std::cout << "HTTP/1.1 200 OK" << Rain::CRLF
		<< "content-type:audio/mpeg" << Rain::CRLF
		//<< "content-range:bytes " << fileBegin << "-" << ((fileEnd == 0) ? "2147483647" : Rain::tToStr(fileEnd)) << "/*" << Rain::CRLF
		<< "accept-ranges:bytes" << Rain::CRLF
		<< Rain::CRLF;
	std::cout.flush();

    //continuously pass mp3 files through cout
	while (true) {
		int id = rand() % 759 + 1;
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(5) << id;
        Rain::readFileToStr("../prototype/" + ss.str() + ".mp3", response);
        std::cout << response;
		response.clear();
        std::cout.flush();
	}

	return 0;
}