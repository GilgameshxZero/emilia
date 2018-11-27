#include "utility-error.h"

namespace Rain {
	int reportError(int code, std::string desc) {
		if (desc != "")
			std::cerr << desc << "\n";

		std::cerr << "Error code: " << code << std::endl;
		return code;
	}
	int errorAndCout(int code, std::string desc, std::string endl) {
		tsCout(desc, endl);
		return reportError(code, desc);
	}

	std::streambuf *redirectCerrFile(std::string filename, bool append) {
		std::ofstream cerrfile;
		cerrfile.open(filename, std::ios_base::out |
					  std::ios_base::binary |
					  (append ? std::ios_base::app : 1));
		return std::cerr.rdbuf(cerrfile.rdbuf());
	}
}