#include "Start.h"

namespace Mono3 {
	namespace SMTPClient {
		int start() {
			std::map<std::string, std::string> config;
			Rain::readParameterFile("config\\config.ini", config);

			//we are being called programmatically probably; initialize config from environment vars/input
			if (config["readConfig"] != "yes") {
				return 1; //todo
			}

			Rain::fastOutputFile(config["logFile"], "", false);

			//find the smtp server of the email host
			std::string emailHost = config["toEmail"];
			std::size_t toEmailDelimPos = emailHost.find("@");
			
			if (toEmailDelimPos == std::string::npos || toEmailDelimPos + 1 >= emailHost.length())
				return -1;
			emailHost = emailHost.substr(toEmailDelimPos + 1, std::string::npos);

			DNS_RECORD *dnsRecord;
			DnsQuery(emailHost.c_str(), DNS_TYPE_MX, DNS_QUERY_STANDARD, NULL, &dnsRecord, NULL);
			std::string smtpServer = dnsRecord->Data.MX.pNameExchange;
			DnsRecordListFree(dnsRecord, DnsFreeRecordList);

			std::cout << "Connecting to " << smtpServer << " for host " << emailHost << "...\r\n";
			Rain::fastOutputFile(config["logFile"], "Connecting to " + smtpServer + " for host " + emailHost + "...\r\n");

			//connect to the smtp server
			WSADATA wsaData;
			struct addrinfo *sAddr;
			SOCKET sSocket;
			if (Rain::quickClientInit(wsaData, smtpServer, config["smtpPort"], &sAddr, sSocket)) {
				Rain::reportError(GetLastError(), "error in quickClientInit");
				return -1;
			}
			if (Rain::connToServ(&sAddr, sSocket)) {
				Rain::reportError(GetLastError(), "error in connToServ");
				return -1;
			}

			//prepare to listen
			RecvThreadParam rtParam;
			Rain::WSA2RecvParam recvParam;

			rtParam.config = &config;
			rtParam.sSocket = &sSocket;
			rtParam.socketActive = true;

			recvParam.bufLen = Rain::strToT<std::size_t>(config["recvBufLen"]);
			recvParam.funcParam = reinterpret_cast<void *>(&rtParam);
			recvParam.message = &rtParam.message;
			recvParam.onProcessMessage = onProcessMessage;
			recvParam.onRecvInit = onRecvInit;
			recvParam.onRecvEnd = onRecvEnd;
			recvParam.socket = &sSocket;

			CreateThread(NULL, 0, Rain::recvThread, reinterpret_cast<void *>(&recvParam), NULL, NULL);

			//stall until smtp done
			while (rtParam.socketActive) {
				rtParam.mainMutex.lock();
				rtParam.mainMutex.unlock();
			}

			Rain::shutdownSocketSend(sSocket);
			closesocket(sSocket);
			WSACleanup();

			std::cout << "EMTSMTPClient has finished. Exiting in 2 seconds...";
			Sleep(2000);

			return 0;
		}
	}
}