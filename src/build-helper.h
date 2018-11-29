#pragma once

#include "rain-aeternum/rain-libraries.h"

//we don't include build.h here; instead, read build.h directly; this speeds up compilation
//#include "../rc/build.h"

#include <fstream>

namespace Emilia {
	std::string getVersionStr();
}