// https://github.com/50c0019uy/cp-library/blob/main/rotate_vector.cpp

#include <cassert>
#include <cstddef>
#include <ranges>
#include <stdexcept>

using namespace std;

using ll = long long;

template<class T>
concept random_access_range_2d =
	std::ranges::random_access_range<T> && std::ranges::random_access_range<typename T::value_type>;

// 1 is right rotate, -1 is left
template<int r, random_access_range_2d T>
requires (r == 1 || r == -1)
[[nodiscard]] T rotate(const T &vv) {
	assert(not vv.empty());

	const size_t h = vv.size();
	const size_t w = vv[0].size();

	for(auto &&v : vv) {
		if(v.size() != w) {
			throw invalid_argument("rotate: Why is arg zig-array!?!?");
		}
	}

	T ret(w, typename T::value_type(h, {}));
	for(size_t i = 0; i < h; i++) {
		for(size_t j = 0; j < w; j++) {
			if constexpr(r == 1) {
				ret[j][h - i - 1] = vv[i][j];
			} else {
				ret[w - j - 1][i] = vv[i][j];
			}
		}
	}

	return ret;
}
