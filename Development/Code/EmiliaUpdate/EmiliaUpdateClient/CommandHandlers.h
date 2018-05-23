#pragma once
#include "../../Common/RainAeternum/RainLibraries.h"

#include <map>
#include <shellapi.h>
#include <string>

namespace Monochrome3 {
	namespace EmiliaUpdateClient {
		typedef int(*CommandMethodHandler)(std::map<std::string, std::string> &);

		//handlers should return nonzero to immediately terminate program
		int CHHelp(std::map<std::string, std::string> &config);
		int CHStageDev(std::map<std::string, std::string> &config);
		int CHDeployStaging(std::map<std::string, std::string> &config);
		int CHProdDownload(std::map<std::string, std::string> &config);
		int CHStageProd(std::map<std::string, std::string> &config);
		int CHProdStop(std::map<std::string, std::string> &config);
		int CHProdStart(std::map<std::string, std::string> &config);
		int CHSyncStop(std::map<std::string, std::string> &config);
		int CHSyncStart(std::map<std::string, std::string> &config);
	}
}