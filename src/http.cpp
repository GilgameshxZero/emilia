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
			// Gets snapshots under a tag, sorted by date.
			{hostRegex,
			 "/api/tags/(.+).json/?" + queryFragment,
			 {Method::GET},
			 &Worker::getApiTagsJson},
			// Gets information for a single snapshot.
			{hostRegex,
			 "/api/snapshots/(.+).json/?" + queryFragment,
			 {Method::GET},
			 &Worker::getApiSnapshotsJson},
			{hostRegex,
			 "/api/snapshots/refresh/?" + queryFragment,
			 {Method::POST},
			 &Worker::getApiSnapshotsRefresh},
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
		return {{StatusCode::OK, {{{"Access-Control-Allow-Origin", "*"}}}}};
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
				 {"Content-Length", std::to_string(ssLen)},
				 {"Access-Control-Allow-Origin", "*"}}},
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
	Worker::ResponseAction Worker::getApiTagsJson(
		Request &,
		std::smatch const &match) {
		std::stringstream ss;
		ss << "{\"snapshots\": [";
		{
			Rain::Multithreading::SharedLockGuard lck(this->server.snapshotsMtx);
			std::vector<std::string> const &tagSnapshots{this->server.tags[match[1]]};
			auto it{tagSnapshots.begin()};
			// Also stream snapshot information so that FE does not need to make
			// multiple requests.
			auto streamSnapshot{
				[this, &ss](std::vector<std::string>::const_iterator it) {
					Snapshot const &snapshot{this->server.snapshots[*it]};
					ss << "{\"name\": \"" << *it << "\", \"title\": \"" << snapshot.title
						 << "\", \"date\": \"" << snapshot.date << "\"}";
				}};
			if (it != tagSnapshots.end()) {
				streamSnapshot(it);
				it++;
			}
			for (; it != tagSnapshots.end(); it++) {
				ss << ",\n";
				streamSnapshot(it);
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
	Worker::ResponseAction Worker::getApiSnapshotsJson(
		Request &,
		std::smatch const &match) {
		auto snapshot{this->server.snapshots[match[1]]};
		std::stringstream ss;
		ss << "{\"name\": \"" << match[1] << "\", \"title\": \"" << snapshot.title
			 << "\", \"date\": \"" << snapshot.date << "\", \"path\": \""
			 << snapshot.path.generic_string() << "\"}";
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
	Worker::ResponseAction Worker::getApiSnapshotsRefresh(
		Request &,
		std::smatch const &) {
		this->server.refreshSnapshots();
		return {{StatusCode::OK, {{{"Access-Control-Allow-Origin", "*"}}}}};
	}
	Worker::ResponseAction Worker::getUserFacing(Request &, std::smatch const &) {
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
		return {
			{StatusCode::OK,
			 {{{"Content-Type", MediaType(path.extension().string())},
				 {"Content-Length", std::to_string(std::filesystem::file_size(path))},
				 // Cached by default.
				 {"Cache-Control", "Max-Age=3600"},
				 {"Access-Control-Allow-Origin", "*"}}},
			 std::move(*file.rdbuf())}};
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
		std::unique_lock lck(this->snapshotsMtx);
		this->tags.clear();
		this->snapshots.clear();

		for (auto const &entry : std::filesystem::recursive_directory_iterator(
					 Server::STATIC_ROOT,
					 std::filesystem::directory_options::follow_directory_symlink)) {
			if (entry.path().extension() != ".html") {
				continue;
			}

			// Parse tags, title, date. These are specified in the HTML file in
			// three lines with some whitespace surrounding.
			std::ifstream fileIStream(entry.path());
			std::string line;
			while (std::getline(fileIStream, line)) {
				if (line != "        <!-- emilia-snapshot-properties") {
					continue;
				}

				// Name is the stem prefixed with the first part of the relative path
				// under `../echidna/`.
				std::string pathStr{entry.path().generic_string()},
					name{
						pathStr.substr(
							Server::STATIC_ROOT.size() + 1,
							pathStr.find('/', Server::STATIC_ROOT.size() + 1) -
								Server::STATIC_ROOT.size() - 1) +
						"-" + entry.path().stem().string()};

				Snapshot &snapshot{this->snapshots[name]};
				if (!snapshot.path.empty()) {
					std::cout << "Duplicate snapshot name: " << name
										<< ". Overwriting...\n";
				}
				snapshot.path = entry.path();
				std::getline(fileIStream, snapshot.title);
				std::getline(fileIStream, snapshot.date);

				// Parse tags
				std::string tagStr, tag;
				std::getline(fileIStream, tagStr);
				std::size_t spacePos, offset{0};
				do {
					spacePos = tagStr.find(' ', offset);
					tag = tagStr.substr(offset, spacePos - offset);
					snapshot.tags.insert(tag);
					this->tags[tag].push_back(name);
					offset = spacePos + 1;
				} while (spacePos != std::string::npos);
				break;
			}
		}

		// Sort snapshots in each tag by their date.
		for (auto &it : this->tags) {
			std::sort(
				it.second.begin(),
				it.second.end(),
				[this](std::string const &a, std::string const &b) {
					return this->snapshots[a].date < this->snapshots[b].date;
				});
		}
	}
}
