// Subclasses Rain::Networking::Http specializations for custom HTTP server.
#include <http.hpp>

#include <emilia.hpp>

namespace Emilia::Http {
	using namespace Rain::Literal;
	using namespace Rain::Networking::Http;

	std::optional<Worker::PreResponse> Worker::chainMatchTargetImpl(
		Tag<Interface>,
		Request &req) {
		std::smatch match;
		if (std::regex_match(req.target, match, ".*"_re)) {
			return this->getAll(req, match);
		}
		return {};
	}

	Worker::PreResponse Worker::getAll(Request &req, std::smatch const &) {
		std::stringstream body;
		body << "emilia " << EMILIA_VERSION_MAJOR << "." << EMILIA_VERSION_MINOR
				 << "." << EMILIA_VERSION_REVISION << "." << EMILIA_VERSION_BUILD
				 << " / rain " << RAIN_VERSION_MAJOR << "." << RAIN_VERSION_MINOR << "."
				 << RAIN_VERSION_REVISION << "." << RAIN_VERSION_BUILD << "\n"
				 << "\n"
				 << "Welcome, "
				 << Rain::Networking::Resolve::getNumericHost(
							this->addressInfo.address, this->addressInfo.addressLen)
				 << ".\n"
				 << "This website is under construction. Please visit later.\n"
				 << "You may reach me at any mailbox under this domain, except for "
						"robots@gilgamesh.cc.\n"
				 << "\n"
				 << "Some links:\n"
				 << "* https://3therflux.bandcamp.com/album/prismaticism\n"
				 << "* https://www.pixiv.net/en/artworks/91277456\n"
				 << "* https://twitter.com/DEMONDICEkaren\n"
				 << "* https://www.youtube.com/watch?v=k_hUdZJNzkU\n"
				 << "* https://github.com/GilgameshxZero/rain\n"
				 << "\n"
				 << "Your request and headers:\n"
				 << req.method << " " << req.target << " HTTP/" << req.version << "\n"
				 << req.headers;
		return {
			StatusCode::OK,
			{},
			{body.str()},
			"Why are you looking here?",
			req.version};
	}
}
