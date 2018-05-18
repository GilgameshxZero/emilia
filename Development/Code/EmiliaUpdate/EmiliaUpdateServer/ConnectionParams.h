#pragma once

#include <map>
#include <string>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		struct ConnectionCallerParam {
			//global config options
			std::map<std::string, std::string> *config;
		};

		struct ConnectionDelegateParam {
			//connection socket (from ltrfdParam)
			SOCKET *cSocket;

			//global config options (from ltrfdParam.ccParam)
			std::map<std::string, std::string> *config;

			//accumulated request from messages
			std::string request;

			//length of the request
			std::size_t requestLength;

			//parsed first section of the request
			std::string requestMethod;

			//whether current socket is authenticated
			bool authenticated;
		};
	}
}