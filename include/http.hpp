// Public interfaces for http.cpp.
#pragma once

#include "state.hpp"

#include <rain.hpp>

namespace Emilia::Http {
	class Worker : public Rain::Networking::Http::Worker<
									 Rain::Networking::Http::Request,
									 Rain::Networking::Http::Response,
									 1 << 10,
									 1 << 10,
									 15000,
									 15000,
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
			15000,
			15000,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::NoLingerSocketOption>;

		State &state;

		// Load from cache or load from filesystem.
		ResponseAction staticFile(
			std::filesystem::path const &,
			std::string const &);

		ResponseAction reqStatus(Request &, std::smatch const &);
		ResponseAction reqHyperspace(Request &, std::smatch const &);
		ResponseAction reqHyperpanel(Request &, std::smatch const &);
		ResponseAction reqPastel(Request &, std::smatch const &);
		ResponseAction reqStarfall(Request &, std::smatch const &);
		ResponseAction reqEutopia(Request &, std::smatch const &);
		virtual std::vector<RequestFilter> filters() override;

		public:
		Worker(NativeSocket, SocketInterface *, State &);

		using SuperWorker::send;
		using SuperWorker::recv;
		virtual void send(Response &) override;
		virtual Request &recv(Request &) override;
	};

	class Server : public Rain::Networking::Http::Server<
									 Worker,
									 Rain::Networking::Ipv6FamilyInterface,
									 Rain::Networking::StreamTypeInterface,
									 Rain::Networking::TcpProtocolInterface,
									 Rain::Networking::DualStackSocketOption,
									 Rain::Networking::NoLingerSocketOption> {
		private:
		using SuperServer = Rain::Networking::Http::Server<
			Worker,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::DualStackSocketOption,
			Rain::Networking::NoLingerSocketOption>;

		State &state;

		public:
		Server(State &, Host const &);
		virtual ~Server();

		private:
		virtual Worker makeWorker(NativeSocket, SocketInterface *) override;
	};
}
