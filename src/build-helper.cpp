#include "build-helper.h"

namespace Emilia {
	std::string getVersionStr() {
		static int major = 0, minor, revision, build;

		//if not called before, read the build file
		if (major == 0) {
			std::ifstream in("..\\rc\\build.h");
			while (in) {
				std::string token;
				in >> token;
				if (token == "VERSION_MAJOR") {
					in >> token;
					major = Rain::strToT<int>(token);
				} else if (token == "VERSION_MINOR") {
					in >> token;
					minor = Rain::strToT<int>(token);
				} else if(token == "VERSION_REVISION") {
					in >> token;
					revision = Rain::strToT<int>(token);
				} else if(token == "VERSION_BUILD") {
					in >> token;
					build = Rain::strToT<int>(token);
				}
			}
			in.close();
		}

		return Rain::tToStr(major) + "." + Rain::tToStr(minor) + "." + Rain::tToStr(revision) + "." + Rain::tToStr(build);
	}
}