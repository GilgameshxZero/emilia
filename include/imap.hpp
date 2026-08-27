#pragma once

#include <rain.hpp>

namespace Emilia::Imap {
	class Worker :
		public Rain::Networking::Imap::Worker<
			Rain::Networking::Imap::Request,
			Rain::Networking::Imap::Response,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::NoLingerSocketOption> {
		public:
		using SuperWorker = Rain::Networking::Imap::Worker<
			Rain::Networking::Imap::Request,
			Rain::Networking::Imap::Response,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::NoLingerSocketOption>;

		public:
	Worker(NativeSocket, SocketInterface *);
	};

	class Client :
		public Rain::Networking::Imap::Client<
			Rain::Networking::Smtp::Request,
			Rain::Networking::Smtp::Response,
			Rain::Networking::Ipv4FamilyInterface,
			Rain::Networking::NoLingerSocketOption> {};

	class Server :
		public Rain::Networking::Imap::Server<
			Worker,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::DualStackSocketOption,
			Rain::Networking::NoLingerSocketOption> {
		public:
		using SuperServer = Rain::Networking::Imap::Server<
			Worker,
			Rain::Networking::Ipv6FamilyInterface,
			Rain::Networking::DualStackSocketOption,
			Rain::Networking::NoLingerSocketOption>;

		using SuperServer::Server;

		public:
		virtual ~Server();

		private:
		virtual Worker makeWorker(
			NativeSocket,
			SocketInterface *) override;
	};
}
