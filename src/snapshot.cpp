#include <snapshot.hpp>

namespace Emilia {
	Snapshot::Snapshot(
		std::filesystem::path const &path,
		std::string const &title,
		std::string const &date,
		std::unordered_set<std::string> const &tags)
			: path(path), title(title), date(date), tags(tags) {}
}
