// Subclasses Rain::Networking::Http specializations for custom HTTP server.
#include <http.hpp>

#include <emilia.hpp>
#include <envelope.hpp>

#include <shared_mutex>

namespace Emilia::Http {
	std::string const Server::STATIC_ROOT{"../static"};
	std::string const Server::HTTP_USERNAME{"gilgamesh"};

	Worker::Worker(
		NativeSocket nativeSocket,
		SocketInterface *interrupter,
		Server &server)
			: SuperWorker(nativeSocket, interrupter), server(server) {}
	void Worker::send(Response &res) {
		// Postprocess to add server signature.
		res.headers.server("emilia " STRINGIFY(EMILIA_VERSION_MAJOR) "." STRINGIFY(EMILIA_VERSION_MINOR) "." STRINGIFY(EMILIA_VERSION_REVISION) "." STRINGIFY(EMILIA_VERSION_BUILD) " / rain " STRINGIFY(RAIN_VERSION_MAJOR) "." STRINGIFY(RAIN_VERSION_MINOR) "." STRINGIFY(RAIN_VERSION_REVISION) "." STRINGIFY(RAIN_VERSION_BUILD));
		if (this->server.echo) {
			// Body is omitted since it can only be transmitted once!
			std::cout << "HTTP to " << this->peerHost() << ":\n"
								<< "HTTP/" << res.version << ' ' << res.statusCode << ' '
								<< res.reasonPhrase << '\n'
								<< res.headers << std::endl;
		}
		SuperWorker::send(res);
	}
	Worker::Request &Worker::recv(Request &req) {
		SuperWorker::recv(req);
		if (this->server.echo) {
			// Body is omitted since it can only be transmitted once!
			std::cout << "HTTP from " << this->peerHost() << ":\n"
								<< req.method << ' ' << req.target << " HTTP/" << req.version
								<< '\n'
								<< req.headers << std::endl;
		}
		return req;
	}
	std::vector<Worker::RequestFilter> const &Worker::filters() {
		// Hosts are gilgamesh.cc, localhost, 127.0.0.1, or ::1. Refer
		// to <https://en.cppreference.com/w/cpp/regex/ecmascript>.
		static std::string const hostRegex{
			"(?:gilgamesh.cc|localhost|127\\.0\\.0\\.1|::1)(?::.*)?"},
			queryFragment{"(?:\\?[^#]*)?(?:#.*)?"};
		static std::string const storyworldsOrRegex{
			"erlija-past|reflections-on-blackfeather"};
		static std::vector<Worker::RequestFilter> const filters{
			// Responds immediately with 200.
			{hostRegex,
			 "/api/ping/?" + queryFragment,
			 {Method::GET},
			 &Worker::getApiPing},
			// Service status.
			{hostRegex,
			 "/api/status/?" + queryFragment,
			 {Method::GET},
			 &Worker::getApiStatus},
			// User-facing endpoints, which resolve to index.html under the current
			// storyworld, or index.html in general.
			{hostRegex,
			 "/(storyworlds|snapshots/(?:[^\\?#\\.]+)|dashboard)?" + queryFragment,
			 {Method::GET},
			 &Worker::getUserFacing},
			// Storyworld-specific endpoints, which may resolve to a shared static.
			{hostRegex,
			 "/(" + storyworldsOrRegex + ")/([^\\?#]*)" + queryFragment,
			 {Method::GET},
			 &Worker::getStoryworldStatic},
			// Shared statics, or 404.
			{hostRegex,
			 "/([^\\?#]*)" + queryFragment,
			 {Method::GET},
			 &Worker::getSharedStatic}};

		return filters;
	}
	Worker::ResponseAction Worker::getApiPing(Request &, std::smatch const &) {
		return {{StatusCode::OK}};
	}
	Worker::ResponseAction Worker::getApiStatus(
		Request &req,
		std::smatch const &) {
		using namespace Rain::Literal;

		if (this->maybeRejectAuthorization(req)) {
			return {
				{StatusCode::UNAUTHORIZED,
				 {{{"Www-Authenticate", "Basic realm=\"api/status\""}}}}};
		}

		std::time_t time =
			std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm timeData;
		Rain::Time::localtime_r(&time, &timeData);

		std::stringstream ss;
		ss << "emilia " STRINGIFY(EMILIA_VERSION_MAJOR) "." STRINGIFY(EMILIA_VERSION_MINOR) "." STRINGIFY(EMILIA_VERSION_REVISION) "." STRINGIFY(EMILIA_VERSION_BUILD) " / rain " STRINGIFY(RAIN_VERSION_MAJOR) "." STRINGIFY(RAIN_VERSION_MINOR) "." STRINGIFY(RAIN_VERSION_REVISION) "." STRINGIFY(RAIN_VERSION_BUILD) << '\n'
			 << "Build: " << (Rain::Platform::isDebug() ? "Debug" : "Release")
			 << ". Platform: " << Rain::Platform::getPlatform() << ".\n"
			 << '\n'
			 << "Hello world, " << this->peerHost() << ". I am " << this->host()
			 << ".\n"
			 << "Server time:  " << std::put_time(&timeData, "%F %T %z") << ".\n"
			 << "Server start: "
			 << std::put_time(&this->server.processBegin, "%F %T %z") << ".\n"
			 << '\n'
			 << "HTTP threads/workers: " << this->server.threads() << " / "
			 << this->server.workers() << ".\n"
			 << "SMTP threads/workers: " << this->server.smtpServer->threads() << " / "
			 << this->server.smtpServer->workers() << ".\n"
			 << '\n'
			 << "Your request:\n"
			 << req.method << ' ' << req.target << ' ' << req.version << '\n'
			 << req.headers << '\n';

		std::size_t bodyLen = 0;
		char buffer[1 << 10];
		while (req.body.read(buffer, sizeof(buffer))) {
			bodyLen += static_cast<std::size_t>(req.body.gcount());
		}
		bodyLen += static_cast<std::size_t>(req.body.gcount());
		ss << "Your request body has length " << bodyLen << ".\n\n";

		{
			std::shared_lock lck(this->server.smtpServer->outboxMtx);
			ss << "SMTP activity (" << this->server.smtpServer->outbox.size()
				 << " total): \n";
			for (auto const &it : this->server.smtpServer->outbox) {
				std::time_t time = std::chrono::system_clock::to_time_t(
					std::chrono::system_clock::now() +
					std::chrono::duration_cast<std::chrono::system_clock::duration>(
						it.attemptTime - std::chrono::steady_clock::now()));
				std::tm timeData;
				Rain::Time::localtime_r(&time, &timeData);
				ss << std::put_time(&timeData, "%F %T %z") << " | ";
				switch (it.status) {
					case Envelope::Status::PENDING:
						ss << "PENDING";
						break;
					case Envelope::Status::RETRIED:
						ss << "RETRIED";
						break;
					case Envelope::Status::FAILURE:
						ss << "FAILURE";
						break;
					case Envelope::Status::SUCCESS:
						ss << "SUCCESS";
						break;
				}
				ss << " | " << it.from.name << '@' << it.from.host << " > "
					 << it.to.name << '@' << it.to.host << '\n';
			}
		}

		ss.seekg(0, std::ios::end);
		std::size_t ssLen = static_cast<std::size_t>(ss.tellg());
		ss.seekg(0, std::ios::beg);
		return {
			{StatusCode::OK,
			 {{{"Content-Type", "text/plain"},
				 {"Content-Length", std::to_string(ssLen)}}},
			 std::move(*ss.rdbuf())}};
	}
	Worker::ResponseAction Worker::getUserFacing(
		Request &req,
		std::smatch const &match) {
		if (match[1] == "dashboard" && this->maybeRejectAuthorization(req)) {
			return {
				{StatusCode::UNAUTHORIZED,
				 {{{"Www-Authenticate", "Basic realm=\"dashboard\""}}}}};
		}

		// Respond with a storyworld index or the shared index, if SRP failed.
		auto file =
			this->resolveStoryworldPath(this->resolveStoryworld(req), "/index.html");
		auto response = this->getStatic(file.value());
		response.response.value().headers["Cache-Control"] = "no-store";
		return response;
	}
	Worker::ResponseAction Worker::getStoryworldStatic(
		Request &,
		std::smatch const &match) {
		auto file = this->resolveStoryworldPath(match[1], match[2]);
		if (!file) {
			return {{StatusCode::NOT_FOUND}};
		}
		return this->getStatic(file.value());
	}
	Worker::ResponseAction Worker::getSharedStatic(
		Request &,
		std::smatch const &match) {
		auto file = this->resolveStoryworldPath("", match[1]);
		if (!file) {
			return {{StatusCode::NOT_FOUND}};
		}
		return this->getStatic(file.value());
	}
	bool Worker::maybeRejectAuthorization(Request &req) {
		static std::string targetCredentials{
			Rain::Networking::Http::Header::Authorization::encodeBasicCredentials(
				Server::HTTP_USERNAME, this->server.httpPassword)};

		auto authorization = req.headers.authorization();
		return !(
			Rain::String::toLower(authorization.scheme) == "basic" &&
			authorization.parameters["credentials"] == targetCredentials);
	}
	std::string Worker::resolveStoryworld(Request &req) {
		auto cookie = req.headers.cookie();
		auto it = cookie.find("storyworld-selected");
		if (it != cookie.end()) {
			return it->second;
		}
		it = cookie.find("storyworld-defaulted");
		if (it != cookie.end()) {
			return it->second;
		}
		return {};
	}
	std::optional<std::filesystem::path> Worker::resolveStoryworldPath(
		std::string const &storyworld,
		std::string const &target) {
		// target does not begin with / and may or may not end with a trailing /.
		//
		// First attempt to resolve the path at ../static/{storyworld}/{path}, then
		// ../static/{path}.
		static auto const resolvePath =
			[](std::string const &pathStr) -> std::optional<std::filesystem::path> {
			std::filesystem::path path{pathStr};
			if (!std::filesystem::exists(path)) {
				return {};
			}
			path = std::filesystem::canonical(path);
			// All files under "../static" are fair game.
			if (!Rain::Filesystem::isSubpath(path, Server::STATIC_ROOT)) {
				return {};
			}
			return {path};
		};
		if (!storyworld.empty()) {
			std::optional<std::filesystem::path> result =
				resolvePath(Server::STATIC_ROOT + "/" + storyworld + "/" + target);
			if (result) {
				return result;
			}
		}
		return resolvePath(Server::STATIC_ROOT + "/" + target);
	}
	Worker::ResponseAction Worker::getStatic(std::filesystem::path const &path) {
		std::ifstream file(path, std::ios::binary);
		std::string bytes;
		bytes.assign(
			(std::istreambuf_iterator<char>(file)),
			(std::istreambuf_iterator<char>()));

		return {
			{StatusCode::OK,
			 {{{"Content-Type", MediaType(path.extension().string())},
				 // Cached by default.
				 {"Cache-Control", "Max-Age=3600"},
				 {"Access-Control-Allow-Origin", "*"}}},
			 std::move(bytes)}};
	}

	Server::Server(
		Host const &host,
		std::string const &httpPassword,
		std::tm const &processBegin,
		std::unique_ptr<Emilia::Smtp::Server> &smtpServer,
		std::atomic_bool const &echo)
			: SuperServer(host),
				httpPassword(httpPassword),
				processBegin(processBegin),
				smtpServer(smtpServer),
				echo(echo) {}
	Server::~Server() {
		this->destruct();
	}
	Worker Server::makeWorker(
		NativeSocket nativeSocket,
		SocketInterface *interrupter) {
		return {nativeSocket, interrupter, *this};
	}
}
