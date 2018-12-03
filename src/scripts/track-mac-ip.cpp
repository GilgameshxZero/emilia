/*
Emilia-tan Script

This script caches a request's MAC address with its IP in a file on the server, and returns the file as well.
*/

#include "../rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
	const static std::string MAC_IP_FILE = "..\\..\\data\\mac-ip.ini";

	_setmode(_fileno(stdout), _O_BINARY);

	std::string response, ip = std::getenv("CLIENT_IP");
	std::map<std::string, std::string> query = Rain::getQueryToMap(std::getenv("QUERY_STRING")),
		mimap = Rain::readParameterFile(MAC_IP_FILE);
	auto mac = query.find("mac");

	if (mac != query.end()) {
		mimap[mac->second] = ip;
		Rain::writeParameterFile(MAC_IP_FILE, mimap);
	}

	response = "Your IP: " + ip + "<br><br>";
	for (auto it = mimap.begin(); it != mimap.end(); it++) {
		response += it->first + ": " + it->second + "<br>";
	}

	std::cout << "HTTP/1.1 200 OK\r\n"
		<< "content-type:text/html\r\n"
		<< "\r\n"
		<< response;

	return 0;
}