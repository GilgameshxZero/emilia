#include "Main.h"

int main() {
	int error = Mono3::start();

	if (error != 0) {
		std::cout << "start returned error code " << error << "\nExiting in 3 seconds...";
		Sleep(3000);
	}

	return error;
}