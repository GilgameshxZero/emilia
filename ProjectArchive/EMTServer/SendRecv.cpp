#include "SendRecv.h"

namespace Rain
{
	DWORD WINAPI RecvThread (LPVOID lpParameter)
	{
		RecvParam *recvparam = reinterpret_cast<RecvParam *>(lpParameter);
		char *buffer = new char[recvparam->buflen];
		int ret;

		// Receive data until the server closes the connection
		do {
			ret = recv (*recvparam->sock, buffer, recvparam->buflen, 0);
			if (ret > 0) //received ret bytes
			{
				*recvparam->message = std::string (buffer, ret);
				if (recvparam->ProcessMessage != NULL)
					if (recvparam->ProcessMessage (recvparam->funcparam))
						break;
			}
			else if (ret == 0)
				break; //connection closed
			else //failure
			{
				ret = WSAGetLastError ();
				break;
			}
		} while (ret > 0);

		delete[] buffer;
		if (recvparam->OnRecvEnd != NULL)
			recvparam->OnRecvEnd (recvparam->funcparam);

		return ret;
	}

	int SendText (SOCKET &sock, const char *cstrtext, long long len)
	{
		long long sent = 0;
		int ret;

		while (sent < len)
		{
			ret = send (sock, cstrtext + sent, static_cast<int>(len - sent), 0);
			if (ret == SOCKET_ERROR)
			{
				ret = WSAGetLastError ();
				return ret;
			}

			sent += ret;
		}

		return 0;
	}
}