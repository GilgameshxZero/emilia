#include "emilia-web.hpp"

int main(int argc, char *argv[], char **envp) {
	Rain::ThreadPool threadPool;
	Rain::HttpServerSocket socket;
	socket.serve("80", &threadPool);

	Rain::Socket::cleanup();
	return 0;
}
