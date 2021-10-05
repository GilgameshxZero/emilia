// Subclasses Rain::Networking::Http specializations for custom HTTP server.
#include <http.hpp>

#include <emilia.hpp>

namespace Emilia::Http {
	Worker::Worker(
		NativeSocket nativeSocket,
		SocketInterface *interrupter,
		State &state)
			: SuperWorker(nativeSocket, interrupter), state(state) {}
	Worker::ResponseAction Worker::staticFile(
		std::filesystem::path const &root,
		std::string const &target) {
		std::filesystem::path path(root / target);
		if (!std::filesystem::exists(path)) {
			return {{StatusCode::NOT_FOUND}};
		}
		path = std::filesystem::canonical(path);
		if (std::filesystem::is_directory(path)) {
			path /= "index.html";
		}
		if (!Rain::Filesystem::isSubpath(path, root)) {
			return {{StatusCode::NOT_FOUND}};
		}

		bool inCache;
		{
			std::shared_lock<std::shared_mutex> lck(this->state.fileCacheMtx);
			inCache = this->state.fileCache.find(path) != this->state.fileCache.end();
		}

		// The cache may be updated before this point, but it is fine if we
		// overwrite it.
		if (!inCache) {
			std::unique_lock<std::shared_mutex> lck(this->state.fileCacheMtx);
			std::ifstream file(path, std::ios::binary);

			// No need to over-reserve.
			auto &it = this->state.fileCache[path];
			it.assign(
				(std::istreambuf_iterator<char>(file)),
				(std::istreambuf_iterator<char>()));
			it.shrink_to_fit();
		}

		std::shared_lock<std::shared_mutex> lck(this->state.fileCacheMtx);
		return {
			{StatusCode::OK,
			 {{{"Content-Type", MediaType(path.extension().string())},
				 // ~17 minute cache.
				 {"Cache-Control", "max-age=1024"},
				 {"Access-Control-Allow-Origin", "*"}}},
			 this->state.fileCache[path]}};
	}
	Worker::ResponseAction Worker::reqStatus(Request &req, std::smatch const &) {
		using namespace Rain::Literal;

		std::time_t time =
			std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm timeData;
		Rain::Time::localtime_r(&time, &timeData);

		std::stringstream ss;
		{
			std::shared_lock lck(this->state.fileCacheMtx);
			ss << this->state.signature << '\n'
				 << "Build: " << (Rain::Platform::isDebug() ? "Debug" : "Release")
				 << ". Platform: " << Rain::Platform::getPlatform() << ".\n"
				 << '\n'
				 << "Hello world, " << this->peerHost() << ". I am " << this->host()
				 << ".\n"
				 << "Server time:  " << std::put_time(&timeData, "%F %T %z") << ".\n"
				 << "Server start: "
				 << std::put_time(&this->state.processBegin, "%F %T %z") << ".\n"
				 << '\n'
				 << "HTTP threads/workers: " << this->state.httpServer->threads()
				 << " / " << this->state.httpServer->workers() << ".\n"
				 << "SMTP threads/workers: " << this->state.smtpServer->threads()
				 << " / " << this->state.smtpServer->workers() << ".\n"
				 << "Cached statics: " << this->state.fileCache.size() << ".\n"
				 << '\n'
				 << "Your request:\n"
				 << req.method << ' ' << req.target << ' ' << req.version << '\n'
				 << req.headers << '\n';
		}

		std::size_t bodyLen = 0;
		char buffer[1 << 10];
		while (req.body.read(buffer, sizeof(buffer))) {
			bodyLen += req.body.gcount();
		}
		bodyLen += req.body.gcount();
		ss << "Your request body has length " << bodyLen << ".\n\n";

		{
			std::shared_lock lck(this->state.outboxMtx);
			ss << "Mailbox activity in the past 72 hours ("
				 << this->state.outbox.size()
				 << " activities total over server lifetime): \n";
			for (auto const &it : this->state.outbox) {
				// Show only outbox from last 3 days.
				if (it.attemptTime < std::chrono::steady_clock::now() - 72h) {
					break;
				}
				std::time_t time =
					std::chrono::system_clock::to_time_t(it.attemptSystemTime);
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
		std::size_t ssLen = ss.tellg();
		ss.seekg(0, std::ios::beg);
		return {
			{StatusCode::OK,
			 {{{"Content-Type", "text/plain"},
				 {"Content-Length", std::to_string(ssLen)}}},
			 std::move(*ss.rdbuf())}};
	}
	Worker::ResponseAction Worker::reqHyperspace(
		Request &,
		std::smatch const &match) {
		static std::filesystem::path const root(
			std::filesystem::canonical("../hyperspace"));
		return this->staticFile(root, match[1].str());
	}
	Worker::ResponseAction Worker::reqHyperpanel(
		Request &,
		std::smatch const &match) {
		static std::filesystem::path const root(
			std::filesystem::canonical("../hyperpanel"));
		return this->staticFile(root, match[1].str());
	}
	Worker::ResponseAction Worker::reqPastel(
		Request &,
		std::smatch const &match) {
		static std::filesystem::path const root(
			std::filesystem::canonical("../pastel"));
		return this->staticFile(root, match[1].str());
	}
	Worker::ResponseAction Worker::reqStarfall(
		Request &,
		std::smatch const &match) {
		static std::filesystem::path const root(
			std::filesystem::canonical("../starfall"));
		return this->staticFile(root, match[1].str());
	}
	Worker::ResponseAction Worker::reqEutopia(
		Request &,
		std::smatch const &match) {
		static std::filesystem::path const root(
			std::filesystem::canonical("../eutopia"));
		return this->staticFile(root, match[1].str());
	}
	std::vector<Worker::RequestFilter> Worker::filters() {
		return {
			{"status\\." + this->state.host.node + "(:.*)?",
			 "/([^\\?#]*)(\\?[^#]*)?(#.*)?",
			 {Method::GET, Method::POST},
			 &Worker::reqStatus},
			{"hyperspace\\." + this->state.host.node + "(:.*)?",
			 "/([^\\?#]*)(\\?[^#]*)?(#.*)?",
			 {Method::GET},
			 &Worker::reqHyperspace},
			{"hyperpanel\\." + this->state.host.node + "(:.*)?",
			 "/([^\\?#]*)(\\?[^#]*)?(#.*)?",
			 {Method::GET},
			 &Worker::reqHyperpanel},
			{"pastel\\." + this->state.host.node + "(:.*)?",
			 "/([^\\?#]*)(\\?[^#]*)?(#.*)?",
			 {Method::GET},
			 &Worker::reqPastel},
			{"starfall\\." + this->state.host.node + "(:.*)?",
			 "/([^\\?#]*)(\\?[^#]*)?(#.*)?",
			 {Method::GET},
			 &Worker::reqStarfall},
			// eutopia.gilgamesh.cc, gilgamesh.cc, localhost, 127.0.0.1, or ::1. Refer
			// to <https://en.cppreference.com/w/cpp/regex/ecmascript>.
			{"(?:(eutopia\\.)?" + this->state.host.node +
				 "|localhost|127\\.0\\.0\\.1|::1)(:.*)?",
			 "/([^\\?#]*)(\\?[^#]*)?(#.*)?",
			 {Method::GET},
			 &Worker::reqEutopia}};
	}
	void Worker::send(Response &res) {
		// Postprocess to add server signature.
		res.headers.server(this->state.signature);
		if (this->state.echo) {
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
		if (this->state.echo) {
			// Body is omitted since it can only be transmitted once!
			std::cout << "HTTP from " << this->peerHost() << ":\n"
								<< req.method << ' ' << req.target << " HTTP/" << req.version
								<< '\n'
								<< req.headers << std::endl;
		}
		return req;
	}

	Server::Server(State &state, Host const &host)
			: SuperServer(host), state(state) {}
	Server::~Server() { this->destruct(); }
	Worker Server::makeWorker(
		NativeSocket nativeSocket,
		SocketInterface *interrupter) {
		return {nativeSocket, interrupter, this->state};
	}
}
