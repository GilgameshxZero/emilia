#pragma once

#include "rain-aeternum/rain-libraries.h"

//we don't include build.h here; instead, read resource from executable directly; this speeds up compilation
//#include "../rc/build.h"

#pragma comment(lib, "Version.lib")

namespace Emilia {
	std::string getVersionStr();
}