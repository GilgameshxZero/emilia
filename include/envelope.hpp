#pragma once

#include <rain.hpp>

namespace Emilia {
	// A single envelope represents a fromMailbox, a toMailbox, and email data
	// stored as a filename. Once the envelope is sent, the email data may be
	// removed.
	class Envelope {
		public:
		static std::chrono::hours const RETRY_WAIT;
		static std::size_t const ATTEMPTS_MAX;

		enum class Status { PENDING = 0, RETRIED, FAILURE, SUCCESS };
		Status status;

		// The N-th attempt this envelope has been tried, and the last time it was
		// tried. For PENDING envelopes, this may be in the future.
		std::size_t attempt;
		std::chrono::steady_clock::time_point attemptTime;

		Rain::Networking::Smtp::Mailbox from, to;

		// The file containing the email data; only those with status PENDING or
		// SUCCESS are guaranteed to be valid files.
		std::filesystem::path data;

		Envelope(
			Status,
			std::size_t,
			std::chrono::steady_clock::time_point,
			Rain::Networking::Smtp::Mailbox const &,
			Rain::Networking::Smtp::Mailbox const &,
			std::filesystem::path const &);

		// Comparison operator provided for std::set.
		bool operator<(Envelope const &) const;
	};
}
