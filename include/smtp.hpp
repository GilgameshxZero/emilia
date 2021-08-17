// Public interfaces for smtp.cpp.
#pragma once

#include "state.hpp"

#include <rain.hpp>

#include <map>

namespace Emilia::Smtp {
	class Worker : public Rain::Networking::Smtp::Worker<
									 Rain::Networking::Smtp::Request,
									 Rain::Networking::Smtp::Response,
									 1 << 10,
									 1 << 10,
									 15000,
									 15000,
									 Rain::Networking::Ipv6FamilyInterface,
									 Rain::Networking::StreamTypeInterface,
									 Rain::Networking::TcpProtocolInterface,
									 Rain::Networking::NoLingerSocketOption> {
		private:
		using SuperWorker = Rain::Networking::Smtp::Worker<
			Rain::Networking::Smtp::Request,
			Rain::Networking::Smtp::Response,
			1 << 10,
			1 << 10,
			15000,
			15000,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::NoLingerSocketOption>;

		State &state;
		bool authenticated = false;

		public:
		Worker(NativeSocket, SocketInterface *, State &);

		using SuperWorker::send;
		using SuperWorker::recv;
		virtual void send(Response &) override;
		virtual Request &recv(Request &) override;

		private:
		virtual bool onInitialResponse() override;
		virtual ResponseAction onHelo(Request &) override;
		virtual ResponseAction onMailMailbox(Mailbox const &) override;
		virtual ResponseAction onRcptMailbox(Mailbox const &) override;
		virtual ResponseAction onDataStream(std::istream &) override;
		virtual ResponseAction onRset(Request &) override;
		virtual ResponseAction onAuthLogin(std::string const &, std::string const &)
			override;
	};

	class Server : public Rain::Networking::Smtp::Server<
									 Worker,
									 Rain::Networking::Ipv6FamilyInterface,
									 Rain::Networking::StreamTypeInterface,
									 Rain::Networking::TcpProtocolInterface,
									 Rain::Networking::DualStackSocketOption,
									 Rain::Networking::NoLingerSocketOption> {
		private:
		using SuperServer = Rain::Networking::Smtp::Server<
			Worker,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::DualStackSocketOption,
			Rain::Networking::NoLingerSocketOption>;

		State &state;

		// Manages the outbox.
		std::thread sender;
		bool closed = false;

		public:
		Server(State &, Host const &);
		virtual ~Server();

		private:
		virtual Worker makeWorker(NativeSocket, SocketInterface *) override;
	};

	class Client : public Rain::Networking::Smtp::Client<
									 Rain::Networking::Smtp::Request,
									 Rain::Networking::Smtp::Response,
									 1 << 10,
									 1 << 10,
									 15000,
									 15000,
									 Rain::Networking::Ipv4FamilyInterface,
									 Rain::Networking::StreamTypeInterface,
									 Rain::Networking::TcpProtocolInterface,
									 Rain::Networking::NoLingerSocketOption> {
		private:
		using SuperClient = Rain::Networking::Smtp::Client<
			Rain::Networking::Smtp::Request,
			Rain::Networking::Smtp::Response,
			1 << 10,
			1 << 10,
			15000,
			15000,
			Rain::Networking::Ipv4FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::NoLingerSocketOption>;

		using SuperClient::SuperClient;

		State &state;

		public:
		Client(
			State &,
			std::vector<std::pair<std::size_t, std::string>> const &);

		using SuperClient::send;
		using SuperClient::recv;
		virtual void send(Request &) override;
		virtual Response &recv(Response &) override;
	};
}
