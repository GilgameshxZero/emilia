/*
Emilia-tan Script

This script uses the internal SMTP server to send an email with the parameters in the GET query.
*/

#include "../rain-aeternum/rain-libraries.h"

struct CSMParam {
	HANDLE dcEvent;
	std::string response;
};

int onConnect(void *param) {
	Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhp = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
	CSMParam &csmp = *reinterpret_cast<CSMParam *>(csmdhp.delegateParam);
	ResetEvent(csmp.dcEvent);
	return 0;
}
int onMessage(void *param) {
	Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhp = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
	CSMParam &csmp = *reinterpret_cast<CSMParam *>(csmdhp.delegateParam);
	csmp.response += *csmdhp.message;
	return 0;
}
int onDisconnect(void *param) {
	Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam &csmdhp = *reinterpret_cast<Rain::ClientSocketManager::ClientSocketManagerDelegateHandlerParam *>(param);
	CSMParam &csmp = *reinterpret_cast<CSMParam *>(csmdhp.delegateParam);
	SetEvent(csmp.dcEvent);
	return 0;
}

int main(int argc, char *argv[]) {
    _setmode(_fileno(stdout), _O_BINARY);

    std::string response;
    std::map<std::string, std::string> query = Rain::getQueryToMap(std::getenv("QUERY_STRING"));

	std::stringstream request;
	request << "EHLO emilia-tan.com\r\n"
		<< "MAIL FROM:<" << query["from"] << ">\r\n"
		<< "RCPT TO:<" << query["to"] << ">\r\n"
		<< "DATA\r\n" << query["data"] << "\r\n.\r\n"
		<< "QUIT\r\n";

	Rain::ClientSocketManager csm;
	CSMParam csmp;
	csmp.dcEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	csm.setEventHandlers(onConnect, onMessage, onDisconnect, &csmp);
	csm.setClientTarget("localhost", 25, 25);
	csm.blockForConnect(INFINITE);
	csm.sendRawMessage(request.str());

	WaitForSingleObject(csmp.dcEvent, INFINITE);
	CloseHandle(csmp.dcEvent);
	response = request.str() + "\r\n" + csmp.response;

    std::cout << "HTTP/1.1 200 OK\r\n"
              << "content-type:text/plain\r\n"
              << "\r\n"
              << response;

    return 0;
}