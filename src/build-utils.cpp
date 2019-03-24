#include "build-utils.h"

namespace Emilia {
	std::string getVersionStr() {
		std::string exe = Rain::getExePath();

		DWORD dummy;
		DWORD dwSize = GetFileVersionInfoSize(exe.c_str(), &dummy);
		if (dwSize == 0) {
			return "0.0.0.0";
		}

		std::vector<BYTE> data(dwSize);
		if (!GetFileVersionInfo(exe.c_str(), NULL, dwSize, &data[0])) {
			return "0.0.0.0";
		}

		// get the name and version strings
		LPVOID pvProductVersion = NULL;
		unsigned int iProductVersionLen = 0;
		if (!VerQueryValue(&data[0], _T("\\StringFileInfo\\040904E4\\ProductVersion"), &pvProductVersion, &iProductVersionLen)) {
			return "0.0.0.0";
		}

		return std::string(reinterpret_cast<LPCSTR>(pvProductVersion), iProductVersionLen);
	}
}