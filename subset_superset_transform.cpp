#include <bit>
#include <concepts>
#include <cstddef>
#include <ranges>

using namespace std;

template<auto op, std::ranges::random_access_range R, class T = std::ranges::range_value_t<R>>
requires std::ranges::sized_range<R> && requires (T a, T b) {
	{ op(a, b) } -> same_as<T>;
}
void subset_transform(R &v) {
	const auto n = v.size();
	assert(n != 0 && (n & (n - 1)) == 0);

	for(size_t i = 0; i < bit_width(n) - 1; i++) {
		for(size_t j = 0; j < n; j++) {
			if(j >> i & 1) {
				v[j] = op(v[j], v[j ^ (1LL << i)]);
			}
		}
	}
}

template<auto op, std::ranges::random_access_range R, class T = std::ranges::range_value_t<R>>
requires std::ranges::sized_range<R> && requires (T a, T b) {
	{ op(a, b) } -> same_as<T>;
}
void superset_transform(R &v) {
	const auto n = v.size();
	assert(n != 0 && (n & (n - 1)) == 0);

	for(size_t i = 0; i < bit_width(n) - 1; i++) {
		for(size_t j = 0; j < n; j++) {
			if(not (j >> i & 1)) {
				v[j] = op(v[j], v[j ^ (1LL << i)]);
			}
		}
	}
}
