// Subclasses Rain::Networking::Http specializations for custom HTTP server.
#include <http.hpp>

#include <emilia.hpp>

namespace Emilia::Http {
	using namespace Rain::Literal;
	using namespace Rain::Networking::Http;

	Worker::Worker(
		Rain::Networking::Resolve::AddressInfo const &addressInfo,
		Rain::Networking::Socket &&socket,
		std::size_t recvBufferLen,
		std::size_t sendBufferLen,
		Duration maxRecvIdleDuration,
		Duration sendOnceTimeoutDuration,
		std::list<std::tuple<
			std::chrono::system_clock::time_point,
			bool,
			Rain::Networking::Smtp::Mailbox,
			Rain::Networking::Smtp::Mailbox>> *mailboxActivity,
		std::mutex *mailboxActivityMtx)
			: Interface(
					addressInfo,
					std::move(socket),
					recvBufferLen,
					sendBufferLen,
					maxRecvIdleDuration,
					sendOnceTimeoutDuration),
				mailboxActivity(*mailboxActivity),
				mailboxActivityMtx(*mailboxActivityMtx) {}

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
				 << "You may reach me at any RFC3696-valid local-part under this "
						"domain, except for canned-pork@gilgamesh.cc.\n"
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
				 << req.headers << "\n"
				 << "SMTP activity since server restart:\n";

		for (auto it = this->mailboxActivity.rbegin();
				 it != this->mailboxActivity.rend();
				 it++) {
			std::time_t time = std::chrono::system_clock::to_time_t(std::get<0>(*it));
			std::tm timeData;
			Rain::Time::localtime_r(&time, &timeData);
			Rain::Networking::Smtp::Mailbox &from(std::get<2>(*it)),
				&to(std::get<3>(*it));
			body << std::put_time(&timeData, "%F %T %z") << " | "
					 << (std::get<1>(*it) ? "Success" : "Failure") << " | "
					 << std::string(from.name.length(), '.') << "@" << from.domain
					 << " > " << std::string(to.name.length(), '.') << "@" << to.domain
					 << "\n";
		}

		return {
			StatusCode::OK,
			{},
			{body.str()},
			"Why are you looking here?",
			req.version};
	}

	Server::Server(
		std::size_t maxThreads,
		Rain::Networking::Specification::ProtocolFamily pf,
		std::size_t recvBufferLen,
		std::size_t sendBufferLen,
		Duration maxRecvIdleDuration,
		Duration sendOnceTimeoutDuration,
		std::list<std::tuple<
			std::chrono::system_clock::time_point,
			bool,
			Rain::Networking::Smtp::Mailbox,
			Rain::Networking::Smtp::Mailbox>> *mailboxActivity,
		std::mutex *mailboxActivityMtx)
			: Interface(
					maxThreads,
					// Relay worker construction arguments.
					pf,
					recvBufferLen,
					sendBufferLen,
					maxRecvIdleDuration,
					sendOnceTimeoutDuration),
				mailboxActivity(*mailboxActivity),
				mailboxActivityMtx(*mailboxActivityMtx) {}

	std::unique_ptr<Worker> Server::workerFactory(
		std::shared_ptr<std::pair<
			Rain::Networking::Socket,
			Rain::Networking::Resolve::AddressInfo>> acceptRes) {
		return std::make_unique<Worker>(
			acceptRes->second,
			std::move(acceptRes->first),
			this->recvBufferLen,
			this->sendBufferLen,
			this->maxRecvIdleDuration,
			this->sendOnceTimeoutDuration,
			&this->mailboxActivity,
			&this->mailboxActivityMtx);
	};
}
