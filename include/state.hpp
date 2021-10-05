// Shared state between HTTP/SMTP servers.
#pragma once

#include "envelope.hpp"

#include <rain.hpp>

#include <shared_mutex>

namespace Emilia {
	class State {
		public:
		// Version information.
		std::string signature;

		// Domain name for HTTP and SMTP. Port should be unspecified.
		Rain::Networking::Host host;

		// Process start timepoint.
		std::tm processBegin;

		// Servers.
		std::unique_ptr<Rain::Networking::ServerSocketSpecInterfaceInterface>
			httpServer, smtpServer;

		// If set, all requests/responses should be echoed to stdout.
		std::atomic_bool echo = false;

		// Cache static files into memory.
		std::shared_mutex fileCacheMtx;
		std::unordered_map<
			std::filesystem::path,
			std::string,
			Rain::Filesystem::HashPath>
			fileCache;

		// SMTP settings.
		Rain::Networking::Smtp::Mailbox smtpForward;
		std::string smtpPassword;

		// Incoming mail is stored in the outbox and sent periodically by a server
		// thread. The key is the last time the mail was attempted, or min if never.
		std::shared_mutex outboxMtx;
		std::condition_variable_any outboxEv;
		std::set<Envelope> outbox;
	};
}
