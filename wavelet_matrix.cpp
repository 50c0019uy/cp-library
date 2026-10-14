// https://github.com/50c0019uy/cp-library/blob/main/wavelet_matrix.cpp

#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <ranges>
#include <utility>
#include <vector>

#include <atcoder/segtree.hpp>

using namespace std;
using namespace atcoder;
using ll = long long;

class bitvector {
	private:
	vector<pair<uint32_t, int32_t>> ps;

	public:
	bitvector() = default;

	bitvector(const vector<uint32_t> &b) {
		ps.resize(b.size() + 1);
		int32_t pcs = 0;
		for(const auto &[i, e] : b | views::enumerate) {
			ps[i] = {e, pcs};
			pcs += popcount(e);
		}
		ps.back() = {0, pcs};
	}

	bitvector(const vector<uint64_t> &b) {
		ps.resize(b.size() * 2 + 1);
		int32_t pcs = 0;
		for(const auto &[i, e] : b | views::enumerate) {
			const uint32_t ed = e & ~uint32_t(0);
			const uint32_t eu = e >> 32;
			ps[2 * i] = {ed, pcs + popcount(ed)};
			ps[2 * i + 1] = {eu, pcs + popcount(e)};
			pcs += popcount(e);
		}
		ps.back() = {0, pcs};
	}

	// [0, i)
	[[nodiscard]] ll sum(const ll i) const {
		assert(i < ps.size() * 32);
		return ps[i / 32].second + popcount(ps[i / 32].first & ((1ULL << (i % 32)) - 1));
	}

	// [l, r)
	[[nodiscard]] ll sum(const ll l, const ll r) const {
		assert(0 <= l);
		assert(l <= r);
		assert(r < ps.size() * 32);
		return sum(r) - sum(l);
	}

	[[nodiscard]] ll get(const ll i) const {
		assert(i < ps.size() * 32);
		return ps[i / 32].first >> (i % 32) & 1;
	}
};

template<int bit_width = 64>
class wavelet_matrix {
	protected:
	vector<uint64_t> src;
	array<bitvector, bit_width> bvs;
	size_t n;

	public:
	static constexpr uint64_t MAX =
		bit_width == 64 ? numeric_limits<uint64_t>::max() : ~(~0ULL << bit_width);

	wavelet_matrix(vector<ll> v) : wavelet_matrix(vector<uint64_t>(v.begin(), v.end())) {}

	wavelet_matrix(vector<uint64_t> v) : src(v), n(v.size()) {
		for(const size_t i : views::iota(0, bit_width) | views::reverse) {
			vector<uint64_t> nv(n);
			vector<uint32_t> bv((n + 31) / 32);
			ll c0 = 0;
			for(size_t j = 0; j < n; j++) {
				bv[j / 32] |= (v[j] >> i & 1) << (j % 32);
				c0 += (v[j] >> i & 1) == 0;
			}
			auto lit = nv.begin();
			auto rit = nv.begin() + c0;
			for(const auto &e : v) {
				if((e >> i & 1) == 0) {
					*(lit++) = e;
				} else {
					*(rit++) = e;
				}
			}
			bvs[i] = std::move(bv);
			v = std::move(nv);
		}
	}

	[[nodiscard]] uint64_t get(ll k) const {
		assert(0 <= k && k < n);
		return src[k];
	}

	// [0, x)
	[[nodiscard]] ll count_less(ll l, ll r, const uint64_t x) const {
		assert(0 <= l);
		assert(l <= r);
		assert(r <= n);
		ll res = 0;
		for(const size_t i : views::iota(0, bit_width) | views::reverse) {
			if(x >> i & 1) {
				res += r - l - bvs[i].sum(l, r);
				l = (n - bvs[i].sum(n)) + bvs[i].sum(l);
				r = (n - bvs[i].sum(n)) + bvs[i].sum(r);
			} else {
				l = (l - bvs[i].sum(l));
				r = (r - bvs[i].sum(r));
			}
		}
		return res;
	}

	// [mi, ma)
	[[nodiscard]] ll count(ll l, ll r, const uint64_t mi, const uint64_t ma) const {
		assert(0 <= l);
		assert(l <= r);
		assert(r <= n);
		assert(mi <= ma);
		return count_less(l, r, ma) - count_less(l, r, mi);
	}

	// 0-origin
	[[nodiscard]] uint64_t get_kth(ll l, ll r, ll k) const {
		assert(0 <= l);
		assert(l <= r);
		assert(r <= n);
		assert(0 <= k && k < r - l);
		uint64_t res = 0;
		for(const size_t i : views::iota(0, bit_width) | views::reverse) {
			if(const ll c0 = r - l - bvs[i].sum(l, r); c0 <= k) {
				k -= c0;
				res |= 1ULL << i;
				l = (n - bvs[i].sum(n)) + bvs[i].sum(l);
				r = (n - bvs[i].sum(n)) + bvs[i].sum(r);
			} else {
				l = (l - bvs[i].sum(l));
				r = (r - bvs[i].sum(r));
			}
		}
		return res;
	}

	// lower_bound, if doesnt exists, return MAX
	[[nodiscard]] uint64_t successor(ll l, ll r, const uint64_t x) const {
		assert(0 <= l);
		assert(l <= r);
		assert(r <= n);

		const ll cnt = count_less(l, r, x);
		if(cnt == r - l) return MAX;
		return get_kth(l, r, cnt);
	}

	// prev(upper_bound), if doesnt exists, return 0
	[[nodiscard]] uint64_t predecessor(ll l, ll r, const uint64_t x) const {
		assert(0 <= l);
		assert(l <= r);
		assert(r <= n);

		const ll cnt = x == MAX ? r - l : count_less(l, r, x + 1);
		if(cnt == 0) return 0;
		return get_kth(l, r, cnt - 1);
	}
};

template<class T, auto op, auto e, int bit_width = 64>
requires requires (T a, T b) {
	{ op(a, b) } -> convertible_to<T>;
	{ e() } -> convertible_to<T>;
}
class wavelet_matrix_prod : public wavelet_matrix<bit_width> {
	private:
	using wavelet_matrix<bit_width>::bvs;
	using wavelet_matrix<bit_width>::n;
	array<segtree<T, op, e>, bit_width> xts;
	segtree<T, op, e> xt_leaf;

	[[nodiscard]] T _rangeprod(
		ll l, ll r, const uint64_t mi, const uint64_t ma, int d, const uint64_t low) const {
		if(l == r) {
			return e();
		}

		if(d == -1) {
			if(mi <= low && low < ma)
				return xt_leaf.prod(l, r);
			else
				return e();
		}

		if(d + 1 < bit_width && (low + (1ULL << (d + 1)) <= mi || ma <= low)) {
			return e();
		}
		if(d + 1 < bit_width && mi <= low && low + (1ULL << (d + 1)) <= ma) {
			return xts[d].prod(l, r);
		}

		const ll l0 = (l - bvs[d].sum(l));
		const ll r0 = (r - bvs[d].sum(r));
		const ll l1 = (n - bvs[d].sum(n)) + bvs[d].sum(l);
		const ll r1 = (n - bvs[d].sum(n)) + bvs[d].sum(r);

		return op(
			_rangeprod(l0, r0, mi, ma, d - 1, low), _rangeprod(l1, r1, mi, ma, d - 1, low | (1ULL << d)));
	}

	public:
	template<class U>
	requires constructible_from<wavelet_matrix<bit_width>, U>
	wavelet_matrix_prod(U &&a, vector<T> x) : wavelet_matrix<bit_width>(std::forward<U>(a)) {
		assert(x.size() == n);
		for(const size_t i : views::iota(0, bit_width) | views::reverse) {
			xts[i] = segtree<T, op, e>(x);
			vector<T> left, right;
			left.reserve(n);
			for(size_t j = 0; j < n; j++) {
				if(bvs[i].get(j) == 0) {
					left.push_back(std::move(x[j]));
				} else {
					right.push_back(std::move(x[j]));
				}
			}
			left.insert(left.end(), right.begin(), right.end());
			x = std::move(left);
		}
		xt_leaf = segtree<T, op, e>(x);
	}

	// [mi, ma)
	[[nodiscard]] T rangeprod(ll l, ll r, uint64_t mi, uint64_t ma) const {
		assert(0 <= l);
		assert(l <= r);
		assert(r <= n);
		assert(mi <= ma);
		return _rangeprod(l, r, mi, ma, bit_width - 1, 0);
	}

	void set(ll k, const T &val) {
		assert(0 <= k && k < n);
		for(const size_t i : views::iota(0, bit_width) | views::reverse) {
			xts[i].set(k, val);
			if(bvs[i].get(k) == 0) {
				k = (k - bvs[i].sum(k));
			} else {
				k = (n - bvs[i].sum(n)) + bvs[i].sum(k);
			}
		}
		xt_leaf.set(k, val);
	}
};
