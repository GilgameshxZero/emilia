#pragma once

#include <string>

namespace Monochrome3 {
	namespace EmiliaUpdateServer {
		struct ConnectionDelegateParam {
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