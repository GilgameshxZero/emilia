#include "emilia-web.hpp"

int main(int argc, char *argv[], char **envp) {
	std::cout << EMILIA_WEB_VERSION_BUILD;
	/*Rain::Thread::ThreadPool threadPool;
	Rain::Memory::BufferPool bufferPool;
	Rain::Networking::HttpServer server(&threadPool, &bufferPool);

	std::mutex statusMtx;
	size_t cSlaves = 0;
	std::function<void()> printStatus = [&]() {
		std::cout << "[" << cSlaves << " slaves]"
							<< " [" << threadPool.getCTasks() << "/"
							<< threadPool.getCFreeThreads() << "/"
							<< threadPool.getCThreads() << " threads]"
							<< " [" << bufferPool.getCFreeBlocks() << "/"
							<< bufferPool.getCBlocks() << "/"
							<< static_cast<int>(bufferPool.getUtil() * 100) << "% buffers]";
	};
	server.onNewSlave = [&](Rain::Networking::HttpServer::Slave *slave) {
		statusMtx.lock();
		cSlaves++;
		printStatus();
		std::cout << " onNewSlave" << std::endl;
		statusMtx.unlock();
	};
	server.onDeleteSlave = [&](Rain::Networking::HttpServer::Slave *slave) {
		statusMtx.lock();
		cSlaves--;
		printStatus();
		std::cout << " onDeleteSlave" << std::endl;
		statusMtx.unlock();
	};
	server.onRequest = [&](Rain::Networking::HttpRequest *req) {
		statusMtx.lock();
		printStatus();
		std::cout << " " << req->method << " " << req->uri << std::endl;
		statusMtx.unlock();

		Rain::Networking::HttpServer::Response res;
		res.headers["content-type"] = "text/html;charset=UTF-8";
		res.setBody(
			"<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta "
			"name=\"viewport\" content=\"width=device-width, "
			"initial-scale=0.8\"><title>emilia-web</title></head><body "
			"class=\"loading\">This page is in development…</body></html>");
		req->socket->send(&res);
	};

	std::cout << "Starting server..." << std::endl;
	server.serve(false);

	while (true) {
		std::string command;
		std::cin >> command;
		if (command == "exit") {
			std::cout << "Shutting down server..." << std::endl;
			server.close();
			break;
		}
	}

	Rain::Networking::Socket::cleanup();*/
	return 0;
}
