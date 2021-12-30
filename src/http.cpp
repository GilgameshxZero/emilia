// Subclasses Rain::Networking::Http specializations for custom HTTP server.
#include <http.hpp>

#include <emilia.hpp>
#include <envelope.hpp>

#include <shared_mutex>

namespace Emilia::Http {
	std::vector<std::string> const Server::storyworlds = {
		"erlija-past",
		"reflections-on-blackfeather"};

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
	std::vector<Worker::RequestFilter> Worker::filters() {
		// Hosts are gilgamesh.cc, localhost, 127.0.0.1, or ::1. Refer
		// to <https://en.cppreference.com/w/cpp/regex/ecmascript>.
		static std::string const hostRegex{
			"(?:gilgamesh.cc|localhost|127\\.0\\.0\\.1|::1)(?::.*)?"},
			queryFragment{"(?:\\?[^#]*)?(?:#.*)?"};

		std::string storyworldsOr = this->server.storyworlds[0];
		for (auto const &storyworld : this->server.storyworlds) {
			storyworldsOr += "|" + storyworld;
		}
		return {
			// Service status.
			{hostRegex,
			 "/api/status/?" + queryFragment,
			 {Method::GET},
			 &Worker::requestApiStatus},
			// Themed storyworld defaults.
			{hostRegex,
			 "/api/storyworlds/defaults/(light|dark)",
			 {Method::GET},
			 &Worker::requestApiStoryworldsDefaults},
			// General-agnostic and dependent public endpoints, which request
			// index.html and may have a storyworld preference.
			{hostRegex,
			 "/(storyworlds|snapshots/(?:[^\\?#\\.]+))?" + queryFragment,
			 {Method::GET},
			 &Worker::requestDependentOrResolution},
			// Force storyworld sessions. Must be updated with any new storyworld IDs.
			{hostRegex,
			 "/(?:" + storyworldsOr +
				 ")(?:/(?:storyworlds|snapshots/(?:[^\\?#\\.]+))?)?" + queryFragment,
			 {Method::GET},
			 &Worker::requestForcedDependent},
			// Endpoints for other statics without storyworld preference.
			{hostRegex,
			 "/([^\\?#]*)" + queryFragment,
			 {Method::GET},
			 &Worker::requestSpecificOrStaticAgnostic}};
	}
	Worker::ResponseAction Worker::requestApiStatus(
		Request &req,
		std::smatch const &) {
		using namespace Rain::Literal;

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
				ss << " | " << std::string(it.from.name.length(), '.') << '@'
					 << it.from.host << " > " << std::string(it.to.name.length(), '.')
					 << '@' << it.to.host << '\n';
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
	Worker::ResponseAction Worker::requestApiStoryworldsDefaults(
		Request &,
		std::smatch const &match) {
		std::string storyworld{
			match[1] == "light" ? "erlija-past" : "reflections-on-blackfeather"};
		return {
			{StatusCode::OK,
			 {{{"Content-Type", "text/plain"},
				 {"Content-Length", std::to_string(storyworld.length())}}},
			 storyworld}};
	}
	Worker::ResponseAction Worker::requestDependentOrResolution(
		Request &req,
		std::smatch const &match) {
		// If a storyworld cookie is set, respond with the index page for that
		// storyworld. Then, check if the path has a preferred storyworld.
		// Otherwise, return with the catch-all index.
		auto storyworld = this->resolveStoryworld(req, match[1]);
		auto file = this->resolveStoryworldPath("/index.html", storyworld.first);
		if (!file) {
			// Should not occur as long as index.html exists for all storyworlds and
			// catch-all.
			return {{StatusCode::NOT_FOUND}};
		}
		auto response = this->requestStatic(file.value());
		if (storyworld.second) {
			// Set preferred storyworld if determined that way. The cookie lasts for
			// the entire session.
			response.response->headers.setCookie(
				{{"storyworld-preferred",
					{storyworld.first.value(), {{"Path", "/"}}}}});
		}
		return response;
	}
	std::pair<std::optional<std::string>, bool> Worker::resolveStoryworld(
		Request &req,
		std::string const &path) {
		auto cookie = req.headers.cookie();
		auto it = cookie.find("storyworld-selected");
		if (it != cookie.end()) {
			return {it->second, false};
		}
		static std::unordered_map<std::string, std::string> const preferences{
			{"snapshots/see-it-wasnt-always-like-this", "erlija-past"}};
		auto preference = preferences.find(path);
		if (preference != preferences.end()) {
			return {preference->second, true};
		}
		it = cookie.find("storyworld-preferred");
		if (it != cookie.end()) {
			return {it->second, false};
		}
		it = cookie.find("storyworld-defaulted");
		if (it != cookie.end()) {
			return {it->second, false};
		}
		return {{}, false};
	}
	std::optional<std::filesystem::path> Worker::resolveStoryworldPath(
		std::string const &path,
		std::optional<std::string> const &storyworld) {
		static std::string const staticRoot{"../static"};

		// path does not begin with / and may or may not end with a trailing /.
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
			if (!Rain::Filesystem::isSubpath(path, staticRoot)) {
				return {};
			}
			return {path};
		};
		if (storyworld) {
			std::optional<std::filesystem::path> result =
				resolvePath(staticRoot + "/" + storyworld.value() + "/" + path);
			if (result) {
				return result;
			}
		}
		return resolvePath(staticRoot + "/" + path);
	}
	Worker::ResponseAction Worker::requestStatic(
		std::filesystem::path const &path) {
		std::ifstream file(path, std::ios::binary);
		std::string bytes;
		bytes.assign(
			(std::istreambuf_iterator<char>(file)),
			(std::istreambuf_iterator<char>()));

		return {
			{StatusCode::OK,
			 {{{"Content-Type", MediaType(path.extension().string())},
				 // No cache by default.
				 {"Cache-Control", "no-store"},
				 {"Access-Control-Allow-Origin", "*"}}},
			 std::move(bytes)}};
	}
	Worker::ResponseAction Worker::requestForcedDependent(
		Request &,
		std::smatch const &) {
		auto file = this->resolveStoryworldPath("/forced.html", {});
		if (!file) {
			return {{StatusCode::NOT_FOUND}};
		}
		return this->requestStatic(file.value());
	}
	Worker::ResponseAction Worker::requestSpecificOrStaticAgnostic(
		Request &req,
		std::smatch const &match) {
		auto storyworld = this->resolveStoryworld(req, match[1]);
		auto file = this->resolveStoryworldPath(match[1], storyworld.first);
		if (!file) {
			return {{StatusCode::NOT_FOUND}};
		}
		auto response = this->requestStatic(file.value());

		// Only select files are cached.
		// TODO: Implement a more robust policy.
		static std::set<std::string> const noCache{
			"index.html",
			"index.css",
			"index.js",
			"colors.css",
			"fonts.css",
			"components/snapshot.html",
			"components/snapshot.css",
			"components/snapshot.js"};
		if (noCache.find(match[1]) == noCache.end()) {
			response.response.value().headers["Cache-Control"] = "Max-Age=3600";
		}
		return response;
	}

	Server::Server(
		Host const &host,
		std::tm const &processBegin,
		std::unique_ptr<Emilia::Smtp::Server> &smtpServer,
		std::atomic_bool const &echo)
			: SuperServer(host),
				processBegin(processBegin),
				smtpServer(smtpServer),
				echo(echo) {}
	Server::~Server() { this->destruct(); }
	Worker Server::makeWorker(
		NativeSocket nativeSocket,
		SocketInterface *interrupter) {
		return {nativeSocket, interrupter, *this};
	}
}
