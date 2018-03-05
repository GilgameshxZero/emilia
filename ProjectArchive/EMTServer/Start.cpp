#include "Start.h"

namespace Mono3
{
	int Start ()
	{
		int error;
		std::string tmp;

		//set up listening server
		WSADATA wsadata;
		struct addrinfo *saddr;
		SOCKET listen;
		std::string port;

		std::cout << "Server port (default HTTP, 80): ";
		//std::getline (std::cin, port);
		std::ifstream in ("config.ini");
		std::getline (in, port);
		in.close ();
		std::cout << port << "\n";

		error = Rain::InitWinsock (wsadata);
		if (error) return error;
		error = Rain::GetServAddr (port, &saddr);
		if (error) return error;
		error = Rain::CreateServLSocket (&saddr, listen);
		if (error) return error;
		error = Rain::BindServLSocket (&saddr, listen); //saddr is freed
		if (error) return error;
		error = Rain::ListenServSocket (listen);
		if (error) return error;

		std::cout << "Listening on port " << port << "...\n";

		//use current thread to capture cin, and spawn client threads with message queues to capture client requests
		ClientThreadParam ctparam;
		ctparam.end = ctparam.beg = NULL;
		ctparam.listensock = &listen;
		ctparam.consolewnd = GetConsoleWindow ();
		ctparam.threadnum = 0;

		CreateThread (NULL, 0, Mono3::ClientThreadProc, reinterpret_cast<LPVOID>(&ctparam), 0, NULL);

		std::string command;
		while (true)
		{
			std::cout << "Accepting commands... \n";
			std::cin >> command;
			std::getline (std::cin, tmp);

			if (command == "exit")
			{
				//first, close the listening socket so that all calls to 'accept' terminate
				closesocket (listen);

				//shutdown threads, socket sends, and WSACleanup
				//send WM_DESTROY to all threads
				ctparam.ctllmutex.lock ();
				CTLLNode *cur = ctparam.beg;
				while (cur != NULL)
				{
					SendMessage (cur->hwnd, WM_DESTROY, 0, 0);
					cur = cur->next;
				}
				ctparam.ctllmutex.unlock ();

				//wait on all threads to terminate
				Sleep (1000);//todo

				WSACleanup ();
				break;
			}
		}

		std::cout << "The server has terminated. Press enter to quit.";
		std::getline (std::cin, tmp);

		return 0;
	}
}