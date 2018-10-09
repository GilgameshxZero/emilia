#include "Main.h"

int main(int argc, char* argv[]) {
	int error = Monochrome3::EmiliaMailServer::start(argc, argv);

	if (error != 0) {
		std::cout << "start returned error code " << error << "\nExiting in 3 seconds...";
		fflush(stdout);
		Sleep(3000);
	}

	return error;
}