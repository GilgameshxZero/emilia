#include <envelope.hpp>

namespace Emilia {
	Envelope::Envelope(
		Status status,
		std::size_t attempt,
		std::chrono::steady_clock::time_point attemptTime,
		Rain::Networking::Smtp::Mailbox const &from,
		Rain::Networking::Smtp::Mailbox const &to,
		std::filesystem::path const &data)
			: status(status),
				attempt(attempt),
				attemptTime(attemptTime),
				from(from),
				to(to),
				data(data) {}
	bool Envelope::operator<(Envelope const &other) const {
		// PENDING envelopes are always sorted first. Everything else is sorted by
		// lastAttempt. Stale envelopes are sorted later in either case.
		if (this->status == Status::PENDING && other.status != Status::PENDING) {
			return true;
		} else if (
			this->status != Status::PENDING && other.status == Status::PENDING) {
			return false;
		} else {
			// Both PENDING or both non-PENDING.
			if (this->attemptTime < other.attemptTime) {
				return false;
			} else if (this->attemptTime > other.attemptTime) {
				return true;
			} else {
				// If both status and attemptTime match, then use data to enforce sort
				// order.
				return this->data > other.data;
			}
		}
	}
}
