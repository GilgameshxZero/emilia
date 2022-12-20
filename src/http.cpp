// Subclasses Rain::Networking::Http specializations for custom HTTP server.
#include <http.hpp>

#include <emilia.hpp>
#include <envelope.hpp>

#include <shared_mutex>

namespace Emilia::Http {
	std::string const Server::STATIC_ROOT{"../echidna"};
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
			// Outbox JSON.
			{hostRegex,
			 "/api/outbox.json/?" + queryFragment,
			 {Method::GET},
			 &Worker::getApiOutboxJson},
			// Reloads tags associated with snapshots.
			{hostRegex,
			 "/api/refresh/?" + queryFragment,
			 {Method::GET},
			 &Worker::getApiRefresh},
			// Gets snapshots under a tag, sorted by date.
			{hostRegex,
			 "/api/snapshots/(.+).json/?" + queryFragment,
			 {Method::GET},
			 &Worker::getApiSnapshotsJson},
			// User-facing endpoints, which resolve to index.html.
			{hostRegex,
			 "/((dashboard|map|timeline|snapshots/(?:[^\\?#\\.]+))/?)?" +
				 queryFragment,
			 {Method::GET},
			 &Worker::getUserFacing},
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
		ss << "Your request body has length " << bodyLen << ".";

		ss.seekg(0, std::ios::end);
		std::size_t ssLen = static_cast<std::size_t>(ss.tellg());
		ss.seekg(0, std::ios::beg);
		return {
			{StatusCode::OK,
			 {{{"Content-Type", "text/plain"},
				 {"Content-Length", std::to_string(ssLen)}}},
			 std::move(*ss.rdbuf())}};
	}
	Worker::ResponseAction Worker::getApiOutboxJson(
		Request &req,
		std::smatch const &) {
		if (this->maybeRejectAuthorization(req)) {
			return {
				{StatusCode::UNAUTHORIZED,
				 {{{"Www-Authenticate", "Basic realm=\"api/outbox.json\""}}}}};
		}

		std::stringstream ss;
		ss << "{\"outbox\": [";
		{
			Rain::Multithreading::SharedLockGuard lck(
				this->server.smtpServer->outboxMtx);
			auto it{this->server.smtpServer->outbox.begin()};
			auto streamEnvelope{[&ss, &it]() {
				std::time_t time = std::chrono::system_clock::to_time_t(
					std::chrono::system_clock::now() +
					std::chrono::duration_cast<std::chrono::system_clock::duration>(
						it->attemptTime - std::chrono::steady_clock::now()));
				std::tm timeData;
				Rain::Time::localtime_r(&time, &timeData);
				ss << "{\"time\": \"" << std::put_time(&timeData, "%F %T %z")
					 << "\", \"status\": \"";
				switch (it->status) {
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
				ss << "\", \"from\": \"" << it->from.name << '@' << it->from.host
					 << "\", \"to\": \"" << it->to.name << '@' << it->to.host << "\"}";
			}};
			if (it != this->server.smtpServer->outbox.end()) {
				streamEnvelope();
				it++;
			}
			for (; it != this->server.smtpServer->outbox.end(); it++) {
				ss << ",\n";
				streamEnvelope();
			}
		}
		ss << "]}";
		ss.seekg(0, std::ios::end);
		std::size_t ssLen = static_cast<std::size_t>(ss.tellg());
		ss.seekg(0, std::ios::beg);
		return {
			{StatusCode::OK,
			 {{{"Content-Type", "application/json"},
				 {"Content-Length", std::to_string(ssLen)},
				 {"Access-Control-Allow-Origin", "*"}}},
			 std::move(*ss.rdbuf())}};
	}
	Worker::ResponseAction Worker::getApiRefresh(
		Request &req,
		std::smatch const &) {
		if (this->maybeRejectAuthorization(req)) {
			return {
				{StatusCode::UNAUTHORIZED,
				 {{{"Www-Authenticate", "Basic realm=\"api/refresh\""}}}}};
		}
		this->server.refreshSnapshots();
		return {{StatusCode::OK}};
	}
	Worker::ResponseAction Worker::getApiSnapshotsJson(
		Request &,
		std::smatch const &match) {
		std::stringstream ss;
		ss << "{\"snapshots\": [";
		{
			Rain::Multithreading::SharedLockGuard lck(this->server.snapshotsMtx);
			auto tagSnapshots{this->server.snapshots[match[1]]};
			auto it{tagSnapshots.begin()};
			auto streamSnapshot{[&ss, &it]() {
				ss << "{\"title\": \"" << it->title << "\", \"date\": \"" << it->date
					 << "\", \"path\": " << it->path << "}";
			}};
			if (it != tagSnapshots.end()) {
				streamSnapshot();
				it++;
			}
			for (; it != tagSnapshots.end(); it++) {
				ss << ",\n";
				streamSnapshot();
			}
		}
		ss << "]}";
		ss.seekg(0, std::ios::end);
		std::size_t ssLen = static_cast<std::size_t>(ss.tellg());
		ss.seekg(0, std::ios::beg);
		return {
			{StatusCode::OK,
			 {{{"Content-Type", "application/json"},
				 {"Content-Length", std::to_string(ssLen)},
				 {"Cache-Control", "Max-Age=1"},
				 {"Access-Control-Allow-Origin", "*"}}},
			 std::move(*ss.rdbuf())}};
	}
	Worker::ResponseAction Worker::getUserFacing(
		Request &req,
		std::smatch const &match) {
		// Respond with the shared index. SRP is handled by frontend.
		return this->getStaticResponse(Server::STATIC_ROOT + "/index.html");
	}
	Worker::ResponseAction Worker::getSharedStatic(
		Request &,
		std::smatch const &match) {
		auto file = this->resolvePath(match[1]);
		if (!file) {
			return {{StatusCode::NOT_FOUND}};
		}
		return this->getStaticResponse(file.value());
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
	std::optional<std::filesystem::path> Worker::resolvePath(
		std::string const &target) {
		// target does not begin with / and may or may not end with a trailing /.
		std::filesystem::path path{Server::STATIC_ROOT + "/" + target};
		if (!std::filesystem::exists(path)) {
			return {};
		}
		// All files under STATIC_ROOT are fair game. Allow symlinks by only
		// comparing absolute paths.
		if (!Rain::Filesystem::isSubpath(path, Server::STATIC_ROOT, false)) {
			return {};
		}
		return {path};
	}
	Worker::ResponseAction Worker::getStaticResponse(
		std::filesystem::path const &path) {
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
				echo(echo) {
		this->refreshSnapshots();
	}
	Server::~Server() {
		this->destruct();
	}
	Worker Server::makeWorker(
		NativeSocket nativeSocket,
		SocketInterface *interrupter) {
		return {nativeSocket, interrupter, *this};
	}
	void Server::refreshSnapshots() {
		std::cout << "Refreshing snapshots..\n";
		std::unique_lock lck(this->snapshotsMtx);
		this->snapshots.clear();
		for (auto const &entry : std::filesystem::directory_iterator(
					 Server::STATIC_ROOT + "/snapshots")) {
			if (entry.path().extension() == ".html") {
				// Parse tags, title, date.
				// These are specified in the HTML file in three lines with no
				// whitespace around:
				// <!-- emilia-snapshot-properties
				// Test Title
				// 2022/12/18
				// test silver
				// emilia-snapshot-properties -->
				std::ifstream fileIStream(entry.path());
				std::string line;
				while (std::getline(fileIStream, line)) {
					if (line == "<!-- emilia-snapshot-properties") {
						std::string title, date, tagStr;
						std::getline(fileIStream, title);
						std::getline(fileIStream, date);
						std::getline(fileIStream, tagStr);
						std::size_t spacePos, offset{0};
						do {
							spacePos = tagStr.find(' ', offset);
							this->snapshots[tagStr.substr(offset, spacePos - offset)]
								.emplace_back(entry.path().generic_string(), title, date);
							offset = spacePos + 1;
						} while (spacePos != std::string::npos);
						std::cout << "Found: " << title << " | " << date << " | "
											<< entry.path() << '\n';
						break;
					}
				}
			}
		}

		// Sort snapshots in each tag by date.
		for (auto &it : this->snapshots) {
			std::sort(it.second.begin(), it.second.end());
		}
		std::cout << "Finished refreshing snapshots.\n";
		std::cout.flush();
	}
}
