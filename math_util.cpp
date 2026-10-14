#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <stdexcept>

using namespace std;
using ll = long long;

template<class T, size_t H, size_t W>
using mat = array<array<T, W>, H>;

template<class T, size_t H1, size_t W1, size_t H2, size_t W2>
auto mul(const mat<T, H1, W1> &a, const mat<T, H2, W2> &b) {
	static_assert(W1 == H2, "W1 must be equal to H2");
	mat<T, H1, W2> res{};
	for(size_t i = 0; i < H1; i++) {
		for(size_t j = 0; j < W2; j++) {
			for(size_t k = 0; k < W1; k++) {
				res[i][j] += a[i][k] * b[k][j];
			}
		}
	}
	return res;
}

template<class T, size_t N>
auto pow(mat<T, N, N> a, ll e) {
	if(e < 0) [[unlikely]]
		throw invalid_argument("pow(): exponent less than zero");
	mat<T, N, N> ret{};
	for(size_t i = 0; i < N; i++) {
		ret[i][i] = 1;
	}
	while(e) {
		if(e % 2) ret = mul(ret, a);
		a = mul(a, a);
		e /= 2;
	}
	return ret;
}
