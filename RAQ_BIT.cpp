// https://github.com/50c0019uy/cp-library/blob/main/RAQ_BIT.cpp

#include <cassert>

#include <atcoder/fenwicktree.hpp>

using namespace std;
using namespace atcoder;

template<class T>
class RAQ_BIT {
	public:
	RAQ_BIT(const int _n) : n(_n), bit0(fenwick_tree<T>(n + 1)), bit1(fenwick_tree<T>(n + 1)) {}

	void add(const int p, const T &x) {
		assert(0 <= p && p < n);
		add(p, p + 1, x);
	}

	void add(const int l, const int r, const T &x) {
		assert(0 <= l);
		assert(l <= r && r <= n);
		bit0.add(l, -x * l);
		bit0.add(r, x * r);
		bit1.add(l, x);
		bit1.add(r, -x);
	}

	T sum(const int l, const int r) {
		assert(0 <= l);
		assert(l <= r && r <= n);
		const T lsum = bit1.sum(0, l) * l + bit0.sum(0, l);
		const T rsum = bit1.sum(0, r) * r + bit0.sum(0, r);
		return rsum - lsum;
	}

	private:
	const int n;
	fenwick_tree<T> bit0, bit1;
};
