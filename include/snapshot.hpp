#pragma once

#include <rain.hpp>

namespace Emilia {
	// Representation of a snapshot/essay on the frontend.
	class Snapshot {
		public:
		std::filesystem::path path;
		std::string title, date;
		std::unordered_set<std::string> tags;

		Snapshot(
			std::filesystem::path const &path = "",
			std::string const &title = "",
			std::string const &date = "",
			std::unordered_set<std::string> const &tags = {});
	};
}
