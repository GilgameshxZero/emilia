#include <snapshot.hpp>

namespace Emilia {
	Snapshot::Snapshot(
		std::filesystem::path const &path,
		std::string const &title,
		std::string const &date)
			: path(path), title(title), date(date) {}

	bool Snapshot::operator<(Snapshot const &other) const {
		return this->date < other.date;
	}
}
