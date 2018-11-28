#include "build-id-helpers.h"

namespace Emilia {
	std::string getVersionStr() {
		return Rain::tToStr(VERSION_MAJOR) + "." + Rain::tToStr(VERSION_MINOR) + "." + Rain::tToStr(VERSION_REVISION) + "." + Rain::tToStr(VERSION_BUILD);
	}
}