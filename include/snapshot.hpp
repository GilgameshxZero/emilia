#pragma once

#include <rain.hpp>

namespace Emilia {
	// Representation of a snapshot/essay on the frontend.
	class Snapshot {
		public:
		std::filesystem::path path;
		std::string title;
		std::string date;

		Snapshot(
			std::filesystem::path const &path,
			std::string const &title,
			std::string const &date);

		// Comparison operator for sorting snapshots by date.
		bool operator<(Snapshot const &) const;
	};
}
