// Public interfaces for http.cpp.
#pragma once

#include <rain.hpp>

namespace Emilia::Http {
	class Worker final : public Rain::Networking::Http::WorkerInterface<
												 Rain::Networking::Http::Socket> {
		public:
		using Interface::WorkerInterface;

		private:
		// Extend target match chain.
		virtual std::optional<PreResponse> chainMatchTargetImpl(
			Tag<Interface>,
			Request &) final override;

		// Target match handlers.
		PreResponse getAll(Request &, std::smatch const &);
	};

	typedef Rain::Networking::Http::
		ServerInterface<Rain::Networking::Http::Socket, Worker>
			Server;
}
