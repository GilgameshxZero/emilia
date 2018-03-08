#include "Start.h"

namespace Mono3 {
	namespace SMTPClient {
		int start() {
			//determine if we should use config from the file, or from stdin
			std::map<std::string, std::string> config;
			Rain::readParameterFile("config\\config.ini", config); //don't need all the parameters just yet, but read it anyway just for readConfig

			//adjust stream based on readConfig parameter
			std::istream *configSS;
			std::ifstream configFIn;
			if (config["readConfig"] != "yes") {
				_setmode(_fileno(stdin), _O_BINARY);
				configSS = &std::cin;
			}
			else {
				configFIn.open("config\\config.ini", std::ios::binary);
				configSS = &configFIn;
			}

			//read from stream the same way
			config.clear();
			std::string key, value;

			//config ends by setting "_configEnd_" to "true"
			while (config["_configEnd_"] != "true") {
				std::getline(*configSS, key, ':');
				Rain::strTrim(key);
				if (key == "emailBodyData") { //special config key
					//determine from config whether we should read email body from a file, or from the config itself
					if (Rain::strToT<int>(config["emailBodyLen"]) == 0) { //read from file
						Rain::readFullFile(config["emailBodyFile"], value);
					} else { //read from stream some number of characters after the ':'
						char *ssBuff = new char[Rain::strToT<std::size_t>(config["emailBodyLen"])];
						configSS->read(ssBuff, Rain::strToT<std::size_t>(config["emailBodyLen"]));
						value = std::string(ssBuff, Rain::strToT<std::size_t>(config["emailBodyLen"]));
						delete[] ssBuff;
					}
				} else {
					std::getline(*configSS, value);
					Rain::strTrim(value);
				}
				config[key] = value;
			}
			if (configFIn.is_open())
				configFIn.close();

			//debugging
			Rain::redirectCerrFile(config["errorLog"]);
			Rain::logMemoryLeaks(config["memoryLeakLog"]);

			//find the smtp server of the email host
			std::string emailHost = config["rcptTo"];
			std::size_t toEmailDelimPos = emailHost.find("@");
			
			if (toEmailDelimPos == std::string::npos || toEmailDelimPos + 1 >= emailHost.length())
				return -1;
			emailHost = emailHost.substr(toEmailDelimPos + 1, std::string::npos);

			DNS_RECORD *dnsRecord;
			DnsQuery(emailHost.c_str(), DNS_TYPE_MX, DNS_QUERY_STANDARD, NULL, &dnsRecord, NULL);
			std::string smtpServer = dnsRecord->Data.MX.pNameExchange;
			DnsRecordListFree(dnsRecord, DnsFreeRecordList);

			//logging
			std::cout << "Connecting to " << smtpServer << " for host " << emailHost << "...\r\n";
			Rain::fastOutputFile(config["logFile"], "Connecting to " + smtpServer + " for host " + emailHost + "...\r\n", true);

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

			//stall until smtp done
			bool clientSuccess = false;
			while (!clientSuccess) {
				//prepare to listen
				RecvThreadParam rtParam;
				Rain::WSA2RecvParam recvParam;

				rtParam.config = &config;
				rtParam.sSocket = &sSocket;
				rtParam.socketActive = true;
				rtParam.clientSuccess = &clientSuccess;

				recvParam.bufLen = Rain::strToT<std::size_t>(config["recvBufLen"]);
				recvParam.funcParam = reinterpret_cast<void *>(&rtParam);
				recvParam.message = &rtParam.message;
				recvParam.onProcessMessage = onProcessMessage;
				recvParam.onRecvInit = onRecvInit;
				recvParam.onRecvEnd = onRecvEnd;
				recvParam.socket = &sSocket;

				CreateThread(NULL, 0, Rain::recvThread, reinterpret_cast<void *>(&recvParam), NULL, NULL);

				while (rtParam.socketActive) {
					rtParam.mainMutex.lock();
					rtParam.mainMutex.unlock();
				}

				if (!clientSuccess) {
					//log
					std::cout << "Did not send mail successfully, trying again...\r\n";
					Rain::fastOutputFile(config["logFile"], "Did not send mail successfully, trying again...\r\n", true);

					Sleep(10000);
				}
			}

			Rain::shutdownSocketSend(sSocket);
			closesocket(sSocket);
			WSACleanup();
			std::cout << "Finished sending email\r\n";
			Rain::fastOutputFile(config["logFile"], "Finished sending email\r\n\r\n", true);

			std::cout << "EMTSMTPClient has finished. Exiting in 2 seconds...";
			Sleep(2000);

			return 0;
		}
	}
}