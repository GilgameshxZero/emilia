// Public interfaces for smtp.cpp.
#pragma once

#include <rain.hpp>

#include "envelope.hpp"

#include <map>
#include <shared_mutex>

namespace Emilia::Smtp {
	class Server;

	class Worker : public Rain::Networking::Smtp::Worker<
									 Rain::Networking::Smtp::Request,
									 Rain::Networking::Smtp::Response,
									 1 << 10,
									 1 << 10,
									 60000,
									 60000,
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
			60000,
			60000,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::NoLingerSocketOption>;

		bool authenticated = false;

		Server &server;

		public:
		Worker(NativeSocket, SocketInterface *, Server &);

		using SuperWorker::send;
		using SuperWorker::recv;
		virtual void send(Response &) override;
		virtual Request &recv(Request &) override;

		private:
		virtual bool onInitialResponse() override;
		virtual ResponseAction onHelo(Request &) override;
		virtual ResponseAction onEhlo(Request &) override;
		virtual ResponseAction onMailMailbox(Mailbox const &) override;
		virtual ResponseAction onRcptMailbox(Mailbox const &) override;
		virtual ResponseAction onDataStream(std::istream &) override;
		virtual ResponseAction onRset(Request &) override;
		virtual ResponseAction onAuthLogin(std::string const &, std::string const &)
			override;
	};

	class Client : public Rain::Networking::Smtp::Client<
									 Rain::Networking::Smtp::Request,
									 Rain::Networking::Smtp::Response,
									 1 << 10,
									 1 << 10,
									 60000,
									 60000,
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
			60000,
			60000,
			Rain::Networking::Ipv4FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::NoLingerSocketOption>;

		using SuperClient::SuperClient;

		Server &server;

		public:
		Client(std::vector<std::pair<std::size_t, std::string>> const &, Server &);

		using SuperClient::send;
		using SuperClient::recv;
		virtual void send(Request &) override;
		virtual Response &recv(Response &) override;
	};

	class Server : public Rain::Networking::Smtp::Server<
									 Worker,
									 Rain::Networking::Ipv6FamilyInterface,
									 Rain::Networking::StreamTypeInterface,
									 Rain::Networking::TcpProtocolInterface,
									 Rain::Networking::DualStackSocketOption,
									 Rain::Networking::NoLingerSocketOption> {
		// Allow state access.
		friend Worker;
		friend Client;

		private:
		using SuperServer = Rain::Networking::Smtp::Server<
			Worker,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::StreamTypeInterface,
			Rain::Networking::TcpProtocolInterface,
			Rain::Networking::DualStackSocketOption,
			Rain::Networking::NoLingerSocketOption>;

		public:
		// Incoming mail is stored in the outbox and sent periodically by a server
		// thread. The key is the last time the mail was attempted, or min if never.
		std::condition_variable_any outboxEv;

		// Allow public access from HTTP endpoint and main management.
		std::shared_mutex outboxMtx;
		std::set<Envelope> outbox;

		private:
		// Manages the outbox.
		std::thread sender;
		std::atomic_bool closed = false;

		// Other state from constructor.
		std::atomic_bool const &echo;
		Rain::Networking::Smtp::Mailbox const &smtpForward;
		std::string const &smtpPassword;

		public:
		Server(
			Host const &,
			std::atomic_bool const &,
			Rain::Networking::Smtp::Mailbox const &,
			std::string const &);
		virtual ~Server();

		private:
		virtual Worker makeWorker(NativeSocket, SocketInterface *) override;
	};
}
