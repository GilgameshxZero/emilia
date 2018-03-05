#include "RainError.h"

namespace Rain {
	int reportError(int code, std::string desc) {
		if (desc != "")
			std::cerr << desc << "\n";
		std::cerr << "Error code: " << code << std::endl;
		return code;
	}

	std::streambuf *redirectCerrFile(std::string filename) {
		static std::ofstream cerrfile;
		cerrfile.open(filename, std::ios_base::out | std::ios_base::binary);
		return std::cerr.rdbuf(cerrfile.rdbuf());
	}
}