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
AR<AR<short, MAX_N>, MAX_N> L, U;
AR<string, MAX_N> G;

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

	short ans = 0;
	RF(i, 0, N) {
		RF(j, 0, N) {
			RF(k, 1, N) {
				if (i + k - 1 >= N || j + k - 1 >= N) {
					continue;
				}
				if (
					k > ans && L[i][j + k - 1] >= k && U[i + k - 1][j + k - 1] >= k &&
					L[i + k - 1][j + k - 1] >= k && U[i + k - 1][j] >= k) {
					ans = k;
				}
			}
		}
	}
	cout << ans << '\n';

	return 0;
}
