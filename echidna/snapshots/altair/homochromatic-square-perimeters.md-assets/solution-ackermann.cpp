// C++ template for coding competitions designed for C++11 support.

// GCC-specific optimizations.
#pragma GCC target("avx2")
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")

#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <system_error>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// User-defined literals.
constexpr std::size_t operator"" _zu(unsigned long long value) {
	return static_cast<std::size_t>(value);
}
std::regex operator"" _re(char const *value, std::size_t) {
	return std::regex(value);
}

// Fast I/O setup and utilities.
class IO {
	public:
	IO() {
		// Redirect I/O to/from files if running locally.
#ifndef ONLINE_JUDGE
		std::freopen("in.txt", "r", stdin);
		std::freopen("out.txt", "w", stdout);
#endif

		// Untie C I/O from C++ I/O. Do not intersperse printf/scanf with cin/cout.
		std::ios_base::sync_with_stdio(false);

		// Untie std::cin. Remember to flush std::cout manually on interactive
		// problems!
		std::cin.tie(nullptr);
	}
};
IO io;

// Union-Find/Disjoint-Set-Union implementation. Near-constant time amortized
// union and find.
//
// Implements path compression and union by rank.
class DisjointSetUnion {
	private:
	// A pair of (is_root, X). If node is root, X stores the size of the
	// cluster. Otherwise, X stores the index of the node’s parent.
	mutable std::vector<std::pair<bool, std::size_t>> nodes;

	public:
	DisjointSetUnion(std::size_t const size) : nodes(size, {true, 1}) {}

	std::size_t find(std::size_t const i) const {
		if (this->nodes[i].first) {
			return i;
		}
		return this->nodes[i].second = this->find(this->nodes[i].second);
	}
	std::size_t rank(std::size_t const i) const {
		return this->nodes[this->find(i)].second;
	}
	void join(std::size_t const i, std::size_t const j) {
		std::size_t pI = this->find(i), pJ = this->find(j);
		if (pI == pJ) {
			return;
		}
		if (this->nodes[pI].second > this->nodes[pJ].second) {
			std::swap(pI, pJ);
		}
		this->nodes[pJ].second += this->nodes[pI].second;
		this->nodes[pI] = {false, pJ};
	}
};

// Shorthand for common types.
using ZU = std::size_t;
using LL = long long;
using ULL = unsigned long long;
using LD = long double;
template <typename First, typename Second>
using PR = std::pair<First, Second>;
template <typename Type>
using VR = std::vector<Type>;
template <typename Type, std::size_t Size>
using AR = std::array<Type, Size>;

// Shorthand for loop in range [from, to).
#define RF(x, from, to) \
	for (long long x = from, _rfDir = from < to ? 1 : -1; x != to; x += _rfDir)

// Imports std scope into global scope; care for name conflicts. Also imports
// literals in std::literals.
using namespace std;

/* ---------------------------- End of template. ---------------------------- */

short const MAX_N = 5000;

// Reusing arrays for memory conservation.
AR<AR<short, MAX_N>, MAX_N> L, U, &L_ = L, &U_ = U, R__, &R_ = L;
AR<string, MAX_N> G;
AR<short, MAX_N> S__;
AR<list<short>, MAX_N + 1> R__radix;

int main(int argc, char const *argv[]) {
	short N;
	cin >> N;
	RF(i, 0, N) {
		cin >> G[i];
		G[i].shrink_to_fit();
	}
	RF(i, 0, N) {
		L[i][0] = G[i][0] - '0';
		U[0][i] = G[0][i] - '0';
	}
	RF(i, 0, N) {
		RF(j, 1, N) { L[i][j] = G[i][j] == '0' ? 0 : L[i][j - 1] + 1; }
	}
	RF(i, 1, N) {
		RF(j, 0, N) { U[i][j] = G[i][j] == '0' ? 0 : U[i - 1][j] + 1; }
	}
	RF(i, 0, N) {
		RF(j, 0, N) { R__[i][j] = min(L[i][j], U[i][j]) - i; }
	}
	RF(i, 0, N) {
		L_[i][N - 1] = G[i][N - 1] - '0';
		U_[N - 1][i] = G[N - 1][i] - '0';
	}
	RF(i, 0, N) {
		RF(j, N - 2, -1) { L_[i][j] = G[i][j] == '0' ? 0 : L_[i][j + 1] + 1; }
	}
	RF(i, N - 2, -1) {
		RF(j, 0, N) { U_[i][j] = G[i][j] == '0' ? 0 : U_[i + 1][j] + 1; }
	}
	RF(i, 0, N) {
		RF(j, 0, N) { R_[i][j] = min(L_[i][j], U_[i][j]); }
	}

	short ans = 0;
	RF(D, -(N - 1), N) {
		auto S_ = DisjointSetUnion(N);
		RF(i, 0, N) { S__[i] = i; }
		RF(i, 0, N) { R__radix[i] = {}; }

		// Radix sort R'' on D.
		RF(i, N - 1, -1) {
			short j = i - D;
			if (j < 0 || j >= N) {
				continue;
			}
			R__radix[1 - R__[i][j]].push_back(i);
		}

		RF(i, N - 1, -1) {
			short j = i - D;
			if (j < 0 || j >= N) {
				continue;
			}

			auto t = 1 - i, t_ = R_[i][j] + i - 1;

			// All R'' on D less than t must be joined in S'.
			while (!R__radix[2 - t].empty()) {
				auto i_ = R__radix[2 - t].back();
				if (i_ > 0) {
					auto g = S__[S_.find(i_ - 1)];
					S_.join(i_, i_ - 1);

					// Update S''.
					S__[S_.find(i_)] = g;
				} else {
					// Pseudo-join the first corner by setting its answer to negative.
					S__[S_.find(i_)] = -1;
				}
				R__radix[2 - t].pop_back();
			}

			// Update ans.
			if (t_ >= 0) {
				ans = max(ans, (short)(S__[S_.find(t_)] - i + 1));
			}
		}
	}
	cout << ans << '\n';

	return 0;
}
