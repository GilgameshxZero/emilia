// Public interfaces for http.cpp.
#pragma once

#include <rain.hpp>

namespace Emilia::Http {
	class Worker final : public Rain::Networking::Http::WorkerInterface<
												 Rain::Networking::Http::Socket> {
		private:
		std::list<std::tuple<
			std::chrono::system_clock::time_point,
			bool,
			Rain::Networking::Smtp::Mailbox,
			Rain::Networking::Smtp::Mailbox>> &mailboxActivity;
		std::mutex &mailboxActivityMtx;

		public:
		Worker(
			Rain::Networking::Resolve::AddressInfo const &,
			Rain::Networking::Socket &&,
			std::size_t = std::size_t(1) << 10,
			std::size_t = std::size_t(1) << 10,
			Duration = std::chrono::seconds(60),
			Duration = std::chrono::seconds(60),
			std::list<std::tuple<
				std::chrono::system_clock::time_point,
				bool,
				Rain::Networking::Smtp::Mailbox,
				Rain::Networking::Smtp::Mailbox>> * = nullptr,
			std::mutex * = nullptr);
		Worker(Worker const &) = delete;
		Worker &operator=(Worker const &) = delete;

		private:
		// Extend target match chain.
		virtual std::optional<PreResponse> chainMatchTargetImpl(
			Tag<Interface>,
			Request &) final override;

		// Target match handlers.
		PreResponse getAll(Request &, std::smatch const &);
	};

	class Server : public Rain::Networking::Http::
									 ServerInterface<Rain::Networking::Http::Socket, Worker> {
		friend Worker;

		private:
		std::list<std::tuple<
			std::chrono::system_clock::time_point,
			bool,
			Rain::Networking::Smtp::Mailbox,
			Rain::Networking::Smtp::Mailbox>> &mailboxActivity;
		std::mutex &mailboxActivityMtx;

		public:
		Server(
			std::size_t = 1024,
			Rain::Networking::Specification::ProtocolFamily =
				Rain::Networking::Specification::ProtocolFamily::INET6,
			std::size_t = std::size_t(1) << 10,
			std::size_t = std::size_t(1) << 10,
			Duration = std::chrono::seconds(60),
			Duration = std::chrono::seconds(60),
			std::list<std::tuple<
				std::chrono::system_clock::time_point,
				bool,
				Rain::Networking::Smtp::Mailbox,
				Rain::Networking::Smtp::Mailbox>> * = nullptr,
			std::mutex * = nullptr);
		Server(Server const &) = delete;
		Server &operator=(Server const &) = delete;

		private:
		virtual std::unique_ptr<Worker> workerFactory(
			std::shared_ptr<std::pair<
				Rain::Networking::Socket,
				Rain::Networking::Resolve::AddressInfo>> acceptRes) override;
	};
}
