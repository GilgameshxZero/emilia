// Public interfaces for smtp.cpp.
#pragma once

#include <rain.hpp>

#include <map>

namespace Emilia::Smtp {
	// A single envelope represents a fromMailbox, a toMailbox, and email data
	// stored as a filename. Once the envelope is sent, the email data may be
	// removed.
	class Envelope {
		public:
		Rain::Networking::Smtp::Mailbox from, to;
		std::filesystem::path file;

		// Number of times the envelope has been attempted to be sent.
		std::size_t cAttempts;
	};

	class Worker final : public Rain::Networking::Smtp::WorkerInterface<
												 Rain::Networking::Smtp::Socket> {
		private:
		Rain::Networking::Smtp::Mailbox const &forwardTo;
		std::string const &domain, &sendAsPassword;

		std::multimap<std::chrono::steady_clock::time_point, Envelope> &outbox;
		std::mutex &outboxMtx;
		std::condition_variable &outboxEv;

		bool authenticated;

		public:
		Worker(
			Rain::Networking::Resolve::AddressInfo const &,
			Rain::Networking::Socket &&,
			std::size_t = std::size_t(1) << 10,
			std::size_t = std::size_t(1) << 10,
			Duration = std::chrono::seconds(60),
			Duration = std::chrono::seconds(60),
			Rain::Networking::Smtp::Mailbox const * = nullptr,
			std::string const * = nullptr,
			std::string const * = nullptr,
			std::multimap<std::chrono::steady_clock::time_point, Envelope> * =
				nullptr,
			std::mutex * = nullptr,
			std::condition_variable * = nullptr);
		Worker(Worker const &) = delete;
		Worker &operator=(Worker const &) = delete;

		private:
		virtual bool onInitialResponse() override;
		virtual PreResponse onHelo(Request &) override;
		virtual PreResponse onMailMailbox(
			Rain::Networking::Smtp::Mailbox const &) override;
		virtual PreResponse onRcptMailbox(
			Rain::Networking::Smtp::Mailbox const &) override;
		virtual PreResponse onDataStream(std::istream &) override;
		virtual PreResponse onRset(Request &) override;
		virtual PreResponse onAuthLogin(std::string const &, std::string const &)
			override;

		virtual void streamOutImpl(Tag<Interface>, Response &) override;
		virtual void streamInImpl(Tag<Interface>, Request &) override;
	};

	class Server final
			: public Rain::Networking::Smtp::
					ServerInterface<Rain::Networking::Smtp::Socket, Worker> {
		private:
		friend Worker;

		Rain::Networking::Smtp::Mailbox const forwardTo;
		std::string const domain, sendAsPassword;

		// Emails are queued onto the Server by Workers. When the outbox is
		// modified, a condition variable is triggered, and the Server attempts to
		// send some emails from the outbox. Envelopes are order by their previous
		// attempt time.
		std::multimap<std::chrono::steady_clock::time_point, Envelope> outbox;
		bool outboxClosed;
		std::mutex outboxMtx;
		std::condition_variable outboxEv;
		std::thread outboxThread;

		public:
		Server(
			std::size_t = 1024,
			Rain::Networking::Specification::ProtocolFamily =
				Rain::Networking::Specification::ProtocolFamily::INET6,
			std::size_t = std::size_t(1) << 10,
			std::size_t = std::size_t(1) << 10,
			Duration = std::chrono::seconds(60),
			Duration = std::chrono::seconds(60),
			Rain::Networking::Smtp::Mailbox const & = {},
			std::string const & = "gilgamesh.cc",
			std::string const & = "");
		Server(Server const &) = delete;
		Server &operator=(Server const &) = delete;
		virtual ~Server();

		private:
		virtual std::unique_ptr<Worker> workerFactory(
			std::shared_ptr<std::pair<
				Rain::Networking::Socket,
				Rain::Networking::Resolve::AddressInfo>> acceptRes) override;
	};
}
