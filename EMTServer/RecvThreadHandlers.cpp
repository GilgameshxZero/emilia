#include "RecvThreadHandlers.h"

namespace Mono3 {
	int ProcClientMess(void *param) {
		RecvThreadParam &rfparam = *reinterpret_cast<RecvThreadParam *>(param);
		int error = 0;

		if (rfparam.waitingPOST) //mqueue is guaranteed empty
		{
			if (rfparam.message.length() + rfparam.POSTmessage.length() < rfparam.POSTlen) {
				rfparam.POSTmessage.append(rfparam.message);
				return 0;
			} else if (rfparam.message.length() + rfparam.POSTmessage.length() == rfparam.POSTlen) {
				rfparam.POSTmessage.append(rfparam.message);
				error = ProcFullMess(&rfparam);
				return error;
			} else //message is longer than remaining POST length
			{
				rfparam.POSTmessage.append(rfparam.message.substr(0, rfparam.POSTlen - rfparam.POSTmessage.length()));
				rfparam.message.substr(rfparam.POSTlen - rfparam.POSTmessage.length(), rfparam.message.length());
				error = ProcFullMess(&rfparam);
				if (error) return error;
			}
		}

		//pop all characters from message into the queue, and only process it when we receive a full message (denoted by /r/n/r/n)
		for (std::size_t a = 0; a < rfparam.message.size(); a++)
			rfparam.mqueue.push(rfparam.message[a]);

		//use perfect hashing rabin-karp to identify /r/n/r/n
		unsigned int curhash = 0;
		static const unsigned int targethash = (static_cast<unsigned int>('\r') << 24) |
			(static_cast<unsigned int>('\n') << 16) |
			(static_cast<unsigned int>('\r') << 8) |
			static_cast<unsigned int>('\n');
		while (!rfparam.mqueue.empty()) {
			rfparam.fmess.push_back(rfparam.mqueue.front());
			curhash = (curhash << 8) | (static_cast<unsigned char>(rfparam.mqueue.front()));
			rfparam.mqueue.pop();

			if (curhash == targethash) {
				error = ProcFullMess(&rfparam);
				rfparam.fmess.clear();
				if (error) break;

				if (rfparam.waitingPOST) //clear queue into POST message
				{
					while (!rfparam.mqueue.empty()) {
						rfparam.POSTmessage.push_back(rfparam.mqueue.front());
						rfparam.mqueue.pop();
					}

					if (rfparam.POSTlen == rfparam.POSTmessage.length()) //all of post message is captured
					{
						error = ProcFullMess(&rfparam);
						if (error) break;
					}
				}
			}
		}

		return error;
	}

	void onRecvThreadEnd(void *funcParam) {
		//client wants to terminate socket
		RecvThreadParam &rfparam = *reinterpret_cast<RecvThreadParam *>(funcParam);

		//use postmessage here because we want the thread of the window to process the message, allowing destroywindow to be called
		//WM_RAINAVAILABLE + 1 is the end message
		PostMessage(rfparam.ctllnode->rainWnd.hwnd, WM_LISTENWNDEND, 0, 0);

		//free WSA2RecvParam here, since recvThread won't need it anymore
		delete &rfparam;
	}

	int ProcFullMess(RecvThreadParam *rfparam) {
		int error;
		const int BUFLEN = 1024;
		std::string tmp, tmp2, requrl, httpver, reqtype;
		std::map<std::string, std::string> headermap;
		std::string reqfile, param;

		if (rfparam->waitingPOST) //we are waiting POST message, and the whole message has arrived now
		{
			//init parameters
			headermap = rfparam->headermap;
			reqtype = rfparam->reqtype;
			requrl = rfparam->requrl;
			httpver = rfparam->httpver;

			rfparam->waitingPOST = false;
		} else {
			//get request info
			std::stringstream ss;
			ss << rfparam->fmess;
			ss >> reqtype >> requrl >> httpver;
			std::getline(ss, tmp);

			while (ss >> tmp) {
				if (tmp.back() == ':')
					tmp.pop_back();
				for (std::size_t a = 0; a < tmp.size(); a++)
					tmp[a] = tolower(tmp[a]);
				while (ss.peek() == ' ')
					ss.get();
				std::getline(ss, tmp2);
				if (tmp2.back() == '\r')
					tmp2.pop_back();
				headermap.insert(make_pair(tmp, tmp2));
			}

			//if we have a POST request, don't process it just yet - wait until all bytes are here, then execute the command
			if (reqtype == "POST") {
				//save the request
				rfparam->headermap = headermap;
				rfparam->reqtype = reqtype;
				rfparam->requrl = requrl;
				rfparam->httpver = httpver;

				//mark the messageproc function into a special state
				rfparam->waitingPOST = true;
				rfparam->POSTmessage.clear();
				rfparam->POSTlen = Rain::strToT<long long>(headermap.find("content-length")->second);

				//return, but signal that we are waiting on more messages from client
				return 0;
			}
		}

		//logging
		struct sockaddr client_addr;
		int client_addr_len = sizeof(client_addr);
		getpeername(*rfparam->sock, &client_addr, &client_addr_len);
		struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_addr;
		struct in_addr ipAddr = pV4Addr->sin_addr;
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
		std::string log = static_cast<std::string>(str) + ": " + reqtype + " " + requrl + "\n";
		Rain::rainCoutF(log);
		Rain::fastOutputFile(rfparam->serverAux + "log.txt", log, true);

		//identify the file pointed to by the URL as well as any parameters
		if (requrl == "/")
			reqfile = "index.html";
		else {
			//cut off file at a '?' if it exists - anything after that is the parameter
			int quesind = 0;
			requrl = requrl.substr(1, requrl.size() - 1);
			for (; quesind < requrl.size(); quesind++)
				if (requrl[quesind] == '?')
					break;
			reqfile = Rain::decodeURL(requrl.substr(0, quesind));
			if (quesind < requrl.size())
				param = Rain::decodeURL(requrl.substr(quesind + 1, requrl.size() - quesind - 1));

			//convert / to \\ in reqfile
			for (int a = 0; a < reqfile.size(); a++)
				if (reqfile[a] == '/')
					reqfile[a] = '\\';
		}

		//depending on the extension of the file, serve it to the browser in different ways
		const std::string rootdir = rfparam->serverRootDir;//"C:\\Main\\Active\\Documents\\Programming\\Rain\\Developing\\Emilia-tan\\Emilia-tan\\Root\\";
		std::string fullpath = rootdir + reqfile, fileext;
		reqfile = Rain::getExePath(fullpath);

		for (std::size_t a = 0; a < reqfile.size(); a++) {
			if (reqfile[reqfile.size() - 1 - a] == '.') {
				fileext = reqfile.substr(reqfile.size() - a, a);
				break;
			}
		}

		for (std::size_t a = 0; a < fileext.length(); a++)
			fileext[a] = tolower(fileext[a]);

		//try to get the right contenttype
		std::string contenttype;
		contenttype = "application/octet-stream";

		//if exe, run it as a cgi script
		if (fileext == "exe") {
			//test that the script exists
			std::ifstream in(fullpath);
			if (in.fail()) {
				in.close();
				std::string headerinfo;
				headerinfo = "HTTP/1.1 404 Not Found\n\n";
				error = Rain::sendText(*rfparam->sock, headerinfo.c_str(), headerinfo.length());
				return 1;
			}

			//set some environment variables for the script
			//append them to current environment block
			std::string tempfile = "tempfile";
			std::string envir;

			envir += "QUERY_STRING=" + param; //might have equals signs with GET requests, so need to encode
			envir.push_back('\0');

			auto it = headermap.find("referer");
			if (it != headermap.end())
				envir += "HTTP_REFERER=" + it->second,
				envir.push_back('\0');

			it = headermap.find("content-length");
			if (it != headermap.end())
				envir += "CONTENT_LENGTH=" + it->second,
				envir.push_back('\0');

			it = headermap.find("host");
			if (it != headermap.end())
				envir += "HTTP_HOST=" + it->second,
				envir.push_back('\0');

			LPCH thisenv = GetEnvironmentStrings();
			int prev = 0;
			for (int i = 0; ; i++) {
				if (thisenv[i] == '\0') {
					envir += std::string(thisenv + prev + 1, thisenv + i);
					envir.push_back('\0');
					prev = i;
					if (thisenv[i + 1] == '\0') {
						break;
					}
				}
			}

			envir.push_back('\0');

			//pipe output of script to a local string
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.bInheritHandle = TRUE;
			sa.lpSecurityDescriptor = NULL;

			HANDLE g_hChildStd_OUT_Rd = NULL, g_hChildStd_OUT_Wr = NULL,
				g_hChildStd_IN_Rd = NULL, g_hChildStd_IN_Wr = NULL;
			error = CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0);
			if (!error) return 1;
			error = SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);
			if (!error) return 1;
			error = CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &sa, 0);
			if (!error) return 1;
			error = SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);
			if (!error) return 1;

			STARTUPINFO sinfo;
			PROCESS_INFORMATION pinfo;
			ZeroMemory(&sinfo, sizeof(sinfo));
			ZeroMemory(&pinfo, sizeof(pinfo));
			sinfo.cb = sizeof(sinfo);
			sinfo.dwFlags |= STARTF_USESTDHANDLES;
			sinfo.hStdOutput = g_hChildStd_OUT_Wr;
			sinfo.hStdInput = g_hChildStd_IN_Rd;
			error = CreateProcess(
				fullpath.c_str(),
				NULL,
				NULL,
				NULL,
				TRUE,
				DETACHED_PROCESS,
				reinterpret_cast<LPVOID>(const_cast<char *>(envir.c_str())),
				fullpath.substr(0, fullpath.length() - reqfile.length()).c_str(),
				&sinfo,
				&pinfo);
			if (!error) return 1;

			//if we are processing a POST request, pipe the request into the in pipe and redirect it to the script
			CHAR chBuf[BUFLEN];
			BOOL bSuccess = FALSE;
			DWORD dwRead, dwWritten;
			if (reqtype == "POST") {
				for (std::size_t a = 0; a < rfparam->POSTlen;) {
					if (a + BUFLEN >= rfparam->POSTlen)
						bSuccess = WriteFile(g_hChildStd_IN_Wr,
											 rfparam->POSTmessage.c_str() + a, static_cast<DWORD>(rfparam->POSTlen - a), &dwWritten, NULL);
					else
						bSuccess = WriteFile(g_hChildStd_IN_Wr,
											 rfparam->POSTmessage.c_str() + a, BUFLEN, &dwWritten, NULL);

					if (!bSuccess) return 1;
					a += dwWritten;
				}
			}
			CloseHandle(g_hChildStd_IN_Wr);

			WaitForInputIdle(pinfo.hProcess, 1000 * 15); //wait a max of 15 secs

			CloseHandle(pinfo.hProcess);
			CloseHandle(pinfo.hThread);
			CloseHandle(g_hChildStd_OUT_Wr);
			CloseHandle(g_hChildStd_IN_Rd);

			//send some preliminary header info
			std::string header = std::string("HTTP/1.1 200 OK\r\n");
			error = Rain::sendText(*rfparam->sock, header.c_str(), header.length());
			if (error) return 1;

			//pass the output of the script in the pipe to the client socket, buffered
			for (;;) {
				bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFLEN, &dwRead, NULL);
				if (!bSuccess || dwRead == 0)
					break;

				error = Rain::sendText(*rfparam->sock, chBuf, dwRead);
				if (error)
					break;
			}

			CloseHandle(g_hChildStd_OUT_Rd);
		} else {
			if (fileext == "pdf")
				contenttype = "application/pdf";
			if (fileext == "aac")
				contenttype = "audio/x-aac";
			if (fileext == "avi")
				contenttype = "video/x-msvideo";
			if (fileext == "bmp")
				contenttype = "image/bmp";
			if (fileext == "torrent")
				contenttype = "application/x-bittorrent";
			if (fileext == "c")
				contenttype = "text/x-c";
			if (fileext == "csv")
				contenttype = "text/csv";
			if (fileext == "gif")
				contenttype = "image/gif";
			if (fileext == "html")
				contenttype = "text/html";
			if (fileext == "ico")
				contenttype = "image/x-icon";
			if (fileext == "java")
				contenttype = "text/x-java-source,java";
			if (fileext == "jpeg")
				contenttype = "image/jpeg";
			if (fileext == "jpg")
				contenttype = "image/jpeg";
			if (fileext == "docx")
				contenttype = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
			if (fileext == "ppt")
				contenttype = "application/vnd.ms-powerpoint";
			if (fileext == "pub")
				contenttype = "application/x-mspublisher";
			if (fileext == "wma")
				contenttype = "audio/x-ms-wma";
			if (fileext == "doc")
				contenttype = "application/msword";
			if (fileext == "mid")
				contenttype = "audio/midi";
			if (fileext == "mpeg")
				contenttype = "video/mpeg";
			if (fileext == "mp4a")
				contenttype = "audio/mp4";
			if (fileext == "mp4")
				contenttype = "video/mp4";
			if (fileext == "png")
				contenttype = "image/png";
			if (fileext == "webm")
				contenttype = "video/webm";
			if (fileext == "tiff")
				contenttype = "image/tiff";
			if (fileext == "txt")
				contenttype = "text/plain";
			if (fileext == "wav")
				contenttype = "audio/x-wav";
			if (fileext == "zip")
				contenttype = "application/zip";

			if (fileext == "mp3")
				contenttype = "audio/mpeg";
			if (fileext == "flac")
				contenttype = "audio/flac";
			if (fileext == "ogg")
				contenttype = "audio/ogg";

			if (fileext == "js")
				contenttype = "application/javascript";
			if (fileext == "css")
				contenttype = "text/css";
			if (fileext == "svg")
				contenttype = "image/svg+xml";

			//try to open in browser, and download if not possible
			std::ifstream in;
			std::string headerinfo;
			long long filelen = 0;
			in.open(fullpath, std::ios::binary);

			if (in.fail()) //404 error
			{
				headerinfo = "HTTP/1.1 404 Not Found\n\n";
				error = Rain::sendText(*rfparam->sock, headerinfo.c_str(), headerinfo.length());
				if (error) return 1;
			} else {
				in.seekg(0, in.end);
				filelen = static_cast<long long>(in.tellg());
				in.seekg(0, in.beg);

				headerinfo = std::string("HTTP/1.1 200 OK\r\n") +
					"Content-Type: " + contenttype + "; charset = UTF-8\r\n" +
					"Content-disposition: inline; filename=" + reqfile + "\r\n" +
					"Content-Encoding: UTF-8\r\n" +
					"Content-Length: " + Rain::tToStr(filelen) + "\r\n" +
					"Server: EMTServer (Monochrome03; Rain) by Yang Yan\r\n" +
					"Accept-Ranges: bytes\r\n" +
					"Connection: close\r\n" +
					"\r\n";
				error = Rain::sendText(*rfparam->sock, headerinfo.c_str(), headerinfo.length());
				if (error) return 1;

				//send the file with buffering
				char buffer[BUFLEN];

				for (int a = 0; a < filelen; a += BUFLEN) {
					if (a + BUFLEN > filelen) {
						in.read(buffer, filelen - a);
						error = Rain::sendText(*rfparam->sock, buffer, filelen - a);
					} else {
						in.read(buffer, BUFLEN);
						error = Rain::sendText(*rfparam->sock, buffer, BUFLEN);
					}
					if (error) return 1;
				}
				in.close();
			}
		}

		return 1; //close connection once a full message has been processed
	}
}