/*
Emilia-tan Script

This script modifies the subscriber list for the mailing list.
*/

#include "rain-aeternum/rain-libraries.h"

int main(int argc, char *argv[]) {
  const static std::string DATA_FILE = "..\\..\\data\\mlist-subs.ini";

  _setmode(_fileno(stdout), _O_BINARY);

  std::string response;
  std::map<std::string, std::string> query =
      Rain::getQueryToMap(std::getenv("QUERY_STRING"));

  std::string action = query["action"], email = query["email"];

  if (email == "") {
    response = "No empty emails";
  } else {
    std::vector<std::string> subs = Rain::readMultilineFile(DATA_FILE);
    std::set<std::string> subSet;
    subSet.insert(subs.begin(), subs.end());
    if (action == "remove") {
      if (subSet.find(email) == subSet.end()) {
        response = "Already unsubscribed";
      } else {
        response = "Unsubscribed";
      }
      subSet.erase(email);
    } else if (action == "add") {
      if (subSet.find(email) == subSet.end()) {
        response = "Subscribed";
      } else {
        response = "Already subscribed";
      }
      subSet.insert(email);
    }
    Rain::printToFile(DATA_FILE, "");
    for (auto it = subSet.begin(); it != subSet.end(); it++) {
      Rain::printToFile(DATA_FILE, *it + Rain::LF, true);
    }
  }

  std::cout << "HTTP/1.1 200 OK" << Rain::CRLF << "content-type:text/html"
            << Rain::CRLF << Rain::CRLF << response;

  return 0;
}
