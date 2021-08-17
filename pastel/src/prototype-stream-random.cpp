/*
Emilia-tan Script

This script continuously transfers mp3 files into cout.
*/

#include "rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);

	const long long maxLen = 1024LL * 1024 * 1024 * 1024;

	std::map<std::string, std::string> query = Rain::getQueryToMap(std::getenv("QUERY_STRING"));

	//allow the user to choose a random seed or specify it
	if (query.find("seed") == query.end()) {
		srand(time(NULL));
	} else {
		srand(Rain::strToT<unsigned int>(query["seed"]));
	}

	//allow the user to specify a start byte
	long long procBytes;
	if (query.find("start") == query.end()) {
		procBytes = 0;
	} else {
		procBytes = -Rain::strToT<long long>(query["start"]);
	}

	//parse range requests
	bool isRangeRequest = false;

	char *rangeCStr = std::getenv("HTTP_RANGE");
	long long rangeBegin = 0,
		rangeEnd = 0;
	if (rangeCStr != NULL) {
		isRangeRequest = true;

		std::string rangeRaw = rangeCStr;
		std::string range = rangeRaw.substr(rangeRaw.find("=") + 1);
		std::size_t rangeDelim = range.find("-");
		rangeBegin = Rain::strToT<long long>(range.substr(0, rangeDelim));
		if (rangeDelim != range.length() - 1) {
			rangeEnd = Rain::strToT<long long>(range.substr(rangeDelim + 1));
		}
	}

	//if no end range specified, just return a default length
	if (rangeEnd == 0) {
		rangeEnd = rangeBegin + maxLen - 1;
	}

	std::string response;
	if (!isRangeRequest) {
		//return an absurdly large content length
		std::cout << "HTTP/1.1 200 OK" << Rain::CRLF;
	} else {
		std::cout << "HTTP/1.1 206 Partial Content" << Rain::CRLF
			<< "content-range:bytes " << rangeBegin << "-" << rangeEnd << "/" << maxLen << Rain::CRLF;
	}
	std::cout << "content-type:audio/mpeg" << Rain::CRLF
		<< "accept-ranges:bytes" << Rain::CRLF
		<< "transfer-encoding:chunked" << Rain::CRLF
		<< Rain::CRLF;
	std::cout.flush();

	//make rangeEnd be exclusive
	rangeEnd++;

    //continuously pass mp3 files through cout
	int id;
	std::cout << std::hex;

	std::ofstream out("prototype-stream-random.log");
	while (true) {
		id = rand() % 759 + 1;
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(5) << id;
		std::size_t responseLen = Rain::getFileSize("../prototype/" + ss.str() + ".mp3");

		out << "begin loop; song " << id << " " << ss.str() << Rain::LF;
		out.flush();

		if (procBytes + static_cast<long long>(responseLen) < rangeBegin) {
			out << "skip song" << Rain::LF;
			out.flush();
			procBytes += responseLen;
			continue;
		}
		
		Rain::readFileToStr("../prototype/" + ss.str() + ".mp3", response);
		if (procBytes < rangeBegin) {
			out << "cutoff " << rangeBegin - procBytes << Rain::LF;
			out.flush();
			response = response.substr(rangeBegin - procBytes);
			procBytes = rangeBegin;
		}

		out << "mid loop; isRangeRequest: " << isRangeRequest << "; procBytes: " << procBytes << "; rangeBegin: " << rangeBegin << "; rangeEnd: " << rangeEnd << Rain::LF;
		out.flush();

		if (isRangeRequest && procBytes >= rangeEnd) {
			out << "exited" << Rain::LF;
			out.flush();
			std::cout << 0 << Rain::CRLF << Rain::CRLF;
			std::cout.flush();
			break;
		} else if (procBytes >= rangeBegin) {
			if (isRangeRequest && response.length() > rangeEnd - procBytes) {
				response = response.substr(0, rangeEnd - procBytes);
			}
			out << "sending " << std::hex << response.length() << std::dec << Rain::LF;
			out.flush();

			procBytes += response.length();
			out << "before cout" << Rain::LF;
			out.flush();
			std::cout << response.length() << Rain::CRLF
				<< response << Rain::CRLF;
			out << "before flush" << Rain::LF;
			out.flush();
			std::cout.flush();

			out << "sent " << response.length() << Rain::LF;
			out.flush();
		}

		response.clear();
	}

	return 0;
}
