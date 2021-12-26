// Public interfaces for http.cpp.
#pragma once

#include "smtp.hpp"

#include <rain.hpp>

namespace Emilia::Http {
	class Server;

	class Worker : public Rain::Networking::Http::Worker<
									 Rain::Networking::Http::Request,
									 Rain::Networking::Http::Response,
									 1 << 10,
									 1 << 10,
									 60000,
									 60000,
									 Rain::Networking::Ipv6FamilyInterface,
									 Rain::Networking::StreamTypeInterface,
									 Rain::Networking::TcpProtocolInterface,
									 Rain::Networking::NoLingerSocketOption> {
		private:
		using SuperWorker = Rain::Networking::Http::Worker<
			Rain::Networking::Http::Request,
			Rain::Networking::Http::Response,
			1 << 10,
			1 << 10,
			60000,
			60000,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::NoLingerSocketOption>;

		public:
		Worker(NativeSocket, SocketInterface *, Server &);

		using SuperWorker::send;
		using SuperWorker::recv;
		virtual void send(Response &) override;
		virtual Request &recv(Request &) override;

		private:
		// Server state.
		Server &server;

		virtual std::vector<RequestFilter> filters() override;
		ResponseAction request(Request &, std::smatch const &);

		// Resolves a path + theme into a filesystem path based on precedence.
		// Any path returned is a valid file.
		std::optional<std::filesystem::path> resolveThemedPath(
			std::string const &,
			std::string const &);

		// Make a response from a static file at a path. Assumes path is resolved
		// and valid.
		ResponseAction requestStatic(std::filesystem::path const &);

		// Server status.
		ResponseAction requestStatus(Request &, std::smatch const &);
	};

	class Server : public Rain::Networking::Http::Server<
									 Worker,
									 Rain::Networking::Ipv6FamilyInterface,
									 Rain::Networking::StreamTypeInterface,
									 Rain::Networking::TcpProtocolInterface,
									 Rain::Networking::DualStackSocketOption,
									 Rain::Networking::NoLingerSocketOption> {
		// Allow state access.
		friend Worker;

		private:
		using SuperServer = Rain::Networking::Http::Server<
			Worker,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::DualStackSocketOption,
			Rain::Networking::NoLingerSocketOption>;

		// State.
		std::tm const &processBegin;
		std::unique_ptr<Emilia::Smtp::Server> &smtpServer;
		std::atomic_bool const &echo;

		public:
		Server(
			Host const &,
			std::tm const &,
			std::unique_ptr<Emilia::Smtp::Server> &,
			std::atomic_bool const &);
		virtual ~Server();

		private:
		virtual Worker makeWorker(NativeSocket, SocketInterface *) override;
	};
}
