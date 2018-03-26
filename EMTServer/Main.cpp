#include "Main.h"

int main() {
	int error = Monochrome3::EMTServer::start();

	if (error != 0) {
		std::cout << "start returned error code " << error << "\nExiting in 3 seconds...";
		Sleep(3000);
	}

	return error;
}