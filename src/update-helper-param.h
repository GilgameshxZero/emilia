#pragma once

#include "rain-aeternum/rain-libraries.h"

namespace Emilia {
	namespace UpdateHelper {
		//variables used by the pulling end of push commands
		struct PullProcParam {
			//state of the push/push-exclusive request
			std::string hrPushState;

			std::size_t totalBytes, currentBytes, curFileLenLeft;

			int cfiles, curFile;
			std::vector<std::string> requested;
			std::vector<time_t> requestedFiletimes;
			std::vector<std::size_t> fileLen;
			std::set<int> unwritable;
			std::set<std::string> noRemove;
		};

		//variables used by the pushing end of push commands
		struct PushProcParam {
			std::string state;
			int cfiles;
			std::vector<std::string> requested;
		};
	}
}