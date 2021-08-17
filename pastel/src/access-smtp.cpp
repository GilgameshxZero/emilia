/*
Emilia-tan Script

This script uses the internal SMTP server to send an email with the parameters in the GET query.
*/

#include "rain-aeternum/rain-libraries.h"

struct CSMParam {
	HANDLE dcEvent;
	std::string response;
};

int onConnect(void *param) {
	Rain::ClientSocketManager::DelegateHandlerParam &csmdhp = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(param);
	CSMParam &csmp = *reinterpret_cast<CSMParam *>(csmdhp.delegateParam);
	ResetEvent(csmp.dcEvent);
	return 0;
}
int onMessage(void *param) {
	Rain::ClientSocketManager::DelegateHandlerParam &csmdhp = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(param);
	CSMParam &csmp = *reinterpret_cast<CSMParam *>(csmdhp.delegateParam);
	csmp.response += *csmdhp.message;
	return 0;
}
int onDisconnect(void *param) {
	Rain::ClientSocketManager::DelegateHandlerParam &csmdhp = *reinterpret_cast<Rain::ClientSocketManager::DelegateHandlerParam *>(param);
	CSMParam &csmp = *reinterpret_cast<CSMParam *>(csmdhp.delegateParam);
	SetEvent(csmp.dcEvent);
	return 0;
}

int main(int argc, char *argv[]) {
    _setmode(_fileno(stdout), _O_BINARY);

    std::string response;
	std::stringstream request;
    std::map<std::string, std::string> query = Rain::getQueryToMap(std::getenv("QUERY_STRING"));

	request << "EHLO emilia-tan.com" << Rain::CRLF
		<< "MAIL FROM:<" << query["from"] << ">" << Rain::CRLF
		<< "RCPT TO:<" << query["to"] << ">" << Rain::CRLF
		<< "DATA" << Rain::CRLF << query["data"] << Rain::CRLF << "." << Rain::CRLF
		<< "QUIT" << Rain::CRLF;

	Rain::ClientSocketManager csm;
	CSMParam csmp;
	csmp.dcEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	csm.setEventHandlers(onConnect, onMessage, onDisconnect, &csmp);
	csm.setClientTarget("localhost", 25, 25);
	csm.blockForConnect(3000);
	if (csm.getSocketStatus() != 0) {
		response += "Failed to connect to local SMTP server. Check the \"Act as Operating\" priviledge?";
	} else {
		csm.sendRawMessage(request.str());

		WaitForSingleObject(csmp.dcEvent, 10000);
		CloseHandle(csmp.dcEvent);
		response = request.str() + Rain::CRLF + csmp.response;
	}

    std::cout << "HTTP/1.1 200 OK" << Rain::CRLF
              << "content-type:text/html" << Rain::CRLF
              << Rain::CRLF
              << response;

    return 0;
}
