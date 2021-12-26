// Subclasses Rain::Networking::Http specializations for custom HTTP server.
#include <http.hpp>

#include <emilia.hpp>
#include <envelope.hpp>

#include <shared_mutex>

namespace Emilia::Http {
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
		return {// Server status is a theme-agnostic endpoint.
						// gilgamesh.cc, localhost, 127.0.0.1, or ::1. Refer
						// to <https://en.cppreference.com/w/cpp/regex/ecmascript>.
						{"(?:gilgamesh.cc|localhost|127\\.0\\.0\\.1|::1)(?::.*)?",
						 "/status/?(?:\\?[^#]*)?(?:#.*)?",
						 {Method::GET},
						 &Worker::requestStatus},
						{"(?:gilgamesh.cc|localhost|127\\.0\\.0\\.1|::1)(?::.*)?",
						 "(/[^\\?#]*)(\\?[^#]*)?(?:#.*)?",
						 {Method::GET},
						 &Worker::request}};
	}
	Worker::ResponseAction Worker::request(
		Request &req,
		std::smatch const &match) {
		// Theme is either specified as a cookie, query parameter, or given a
		// server-side default.
		Rain::Networking::Http::QueryParams queryParams(match[2]);
		std::string theme{queryParams["theme"]};
		if (theme.empty()) {
			theme = req.headers.cookie()["theme"];
		}
		if (theme.empty()) {
			// Server-side default theme.
			theme = "erlija-past";
		}

		// Request the theme-associated static.
		std::optional<std::filesystem::path> file =
			this->resolveThemedPath(match[1], theme);
		if (!file) {
			return {{StatusCode::NOT_FOUND}};
		}
		return this->requestStatic(file.value());
	}
	std::optional<std::filesystem::path> Worker::resolveThemedPath(
		std::string const &path,
		std::string const &theme) {
		static std::string const staticRoot{"../static"};

		// path begins with / and may or may not end with a trailing /.
		//
		// First attempt to resolve the path at ../static/theme/{path}, then
		// ../static/{path}.
		static auto const resolvePath =
			[](std::string const &pathStr) -> std::optional<std::filesystem::path> {
			std::filesystem::path path{pathStr};
			// If neither file nor directory exists, resolve with `.html` appended.
			if (!std::filesystem::exists(path)) {
				path += ".html";
			}
			if (!std::filesystem::exists(path)) {
				return {};
			}
			path = std::filesystem::canonical(path);
			if (std::filesystem::is_directory(path)) {
				path /= "index.html";
			}
			// All files under "../static" are fair game.
			if (!Rain::Filesystem::isSubpath(path, staticRoot)) {
				return {};
			}
			return {path};
		};

		std::optional<std::filesystem::path> result =
			resolvePath(staticRoot + "/" + theme + path);
		if (result) {
			return result;
		}
		return resolvePath(staticRoot + path);
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
				 // ~17 minute cache.
				 {"Cache-Control", "max-age=1024"},
				 {"Access-Control-Allow-Origin", "*"}}},
			 std::move(bytes)}};
	}
	Worker::ResponseAction Worker::requestStatus(
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
