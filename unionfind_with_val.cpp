// https://github.com/50c0019uy/cp-library/blob/main/unionfind_with_val.cpp

#include <cassert>
#include <concepts>
#include <utility>
#include <vector>

#include "atcoder/dsu.hpp"

using namespace std;
using namespace atcoder;

using ll = long long;

/*
 * meld()は第一引数に第二引数を併合する関数.
 */
template<class T, auto meld>
requires swappable<T> && regular_invocable<decltype(meld), T &, T &>
class dsu_val : public dsu {
	public:
	dsu_val(const int n, const T &val = {}) : dsu(n), v(vector<T>(n, val)) {}

	template<convertible_to<T> U>
	dsu_val(const int n, const U &val) : dsu(n), v(vector<T>(n, T(val))) {}

	dsu_val(const vector<T> &_v) : dsu(_v.size()), v(_v) {}

	int merge(const int a, const int b) {
		if(same(a, b)) return leader(a);
		const int RA = leader(a);
		const int RB = leader(b);
		meld(v[RA], v[RB]);
		const int NR = dsu::merge(a, b);
		if(NR != RA) swap(v[NR], v[RA]);
		return NR;
	}

	T &operator[](const int a) {
		return v[leader(a)];
	}

	private:
	vector<T> v;
};
