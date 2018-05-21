#pragma once

#include <string>
#include <Windows.h>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		struct ServerStatus {
			//file location of server//absolute or full path
			std::string path;

			//status of server
			//running or stopped
			std::string status;

			//handle to process
			HANDLE process;

			//handles to stdio
			HANDLE stdInRd, stdInWr, stdOutRd, stdOutWr;
		};
	}
}