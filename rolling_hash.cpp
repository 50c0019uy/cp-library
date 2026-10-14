// https://github.com/50c0019uy/cp-library/blob/main/rolling_hash.cpp

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <ranges>
#include <utility>
#include <vector>

#include "atcoder/segtree.hpp"

using namespace std;
using namespace atcoder;
using ull = unsigned long long;

class rolling_hash_dynamic;

// 参考 https://qiita.com/keymoon/items/11fac5627672a6d6a9f6

// mod定数(2^61-1), r2つ.
class rolling_hash {
	public:
	struct hash_type {
		array<ull, 2> h;
		int l;

		auto operator<=>(const hash_type &a) const noexcept {
			return h <=> a.h;
		}

		bool operator==(const hash_type &) const noexcept = default;

		hash_type operator+(const hash_type &a) const {
			hash_type res;
			res.h[0] = (mul(h[0], r1p[a.l]) + a.h[0]) % MOD;
			res.h[1] = (mul(h[1], r2p[a.l]) + a.h[1]) % MOD;
			res.l = l + a.l;
			return res;
		}
	};

	friend rolling_hash_dynamic;

	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, ull>
	rolling_hash(R &&r) {
		if constexpr(std::ranges::sized_range<R>) {
			const auto n = std::ranges::size(r);
			hash1.reserve(n + 1);
			hash2.reserve(n + 1);
		}

		hash1.push_back(0);
		hash2.push_back(0);
		for(const auto &e : r) {
			hash1.push_back((mul(hash1.back(), r1) + e) % MOD);
			hash2.push_back((mul(hash2.back(), r2) + e) % MOD);
		}
	}

	// [L, R)
	[[nodiscard]] hash_type hash(const int l, const int r) const {
		assert(0 <= l);
		assert(l <= r);
		assert(r < (int)hash1.size());
		return hash_type{.h = {(hash1[r] - mul(hash1[l], powm(r1, r - l)) + MOD) % MOD,
											 (hash2[r] - mul(hash2[l], powm(r2, r - l)) + MOD) % MOD},
			.l = r - l};
	}

	private:
	static ull mul(const ull a, const ull b) noexcept {
		constexpr ull MASK31 = (1LL << 31) - 1;
		constexpr ull MASK30 = (1LL << 30) - 1;
		const ull au = a >> 31;
		const ull ad = a & MASK31;
		const ull bu = b >> 31;
		const ull bd = b & MASK31;
		const ull mid = ad * bu + au * bd;
		const ull midu = mid >> 30;
		const ull midd = mid & MASK30;
		return (au * bu * 2 + midu + (midd << 31) + ad * bd) % MOD;
	}

	// a**b mod MOD
	static ull powm(ull a, ull b) noexcept {
		a %= MOD;
		ull res = 1;
		if(a == r1) {
			if(b < tab_size) [[likely]]
				return r1p[b];
			res = r1p.back();
			b -= tab_size - 1;
		}
		if(a == r2) {
			if(b < tab_size) [[likely]]
				return r2p[b];
			res = r2p.back();
			b -= tab_size - 1;
		}
		while(b) {
			if(b & 1) res = mul(res, a);
			a = mul(a, a);
			b >>= 1;
		}
		return res;
	}

	static constexpr ull MOD = (1LL << 61) - 1;
	static const ull r1, r2;
	static constexpr size_t tab_size = 1LL << 20;
	static const array<ull, rolling_hash::tab_size> r1p, r2p;
	vector<ull> hash1, hash2;
};

const ull rolling_hash::r1 = randint(2U, rolling_hash::MOD - 2);
const ull rolling_hash::r2 = randint(2U, rolling_hash::MOD - 2);

const array<ull, rolling_hash::tab_size> rolling_hash::r1p = [] {
	array<ull, rolling_hash::tab_size> r1p;
	r1p[0] = 1;
	for(size_t i = 1; i < r1p.size(); i++) {
		r1p[i] = mul(r1p[i - 1], r1);
	}
	return r1p;
}();
const array<ull, rolling_hash::tab_size> rolling_hash::r2p = [] {
	array<ull, rolling_hash::tab_size> r2p;
	r2p[0] = 1;
	for(size_t i = 1; i < r2p.size(); i++) {
		r2p[i] = mul(r2p[i - 1], r2);
	}
	return r2p;
}();

class rolling_hash_dynamic {
	private:
	struct monoid {
		array<ull, 2> h;
		array<ull, 2> hr;
		int l;

		auto operator<=>(const monoid &a) const noexcept {
			return h <=> a.h;
		}

		bool operator==(const monoid &) const noexcept = default;

		monoid operator+(const monoid &a) const noexcept {
			const auto &r1 = rolling_hash::r1;
			const auto &r2 = rolling_hash::r2;
			const auto &MOD = rolling_hash::MOD;
			monoid res{};
			for(size_t i = 0; i < 2; i++) {
				const auto r = i == 0 ? r1 : r2;
				res.h[i] = (rolling_hash::mul(h[i], rolling_hash::powm(r, a.l)) + a.h[i]) % MOD;
				res.hr[i] = (rolling_hash::mul(a.hr[i], rolling_hash::powm(r, l)) + hr[i]) % MOD;
			}
			res.l = l + a.l;
			return res;
		}

		[[nodiscard]] monoid reversed() const noexcept {
			monoid res(*this);
			swap(res.h, res.hr);
			return res;
		}
	};

	static constexpr auto op = plus<>{};

	static constexpr auto id = [] { return monoid{.h{0, 0}, .hr{0, 0}, .l = 0}; };

	segtree<monoid, op, id> t;

	public:
	template<std::ranges::sized_range R>
	requires std::ranges::input_range<R> && convertible_to<std::ranges::range_value_t<R>, ull>
	rolling_hash_dynamic(R &&r) {
		const size_t n = std::ranges::size(r);

		vector<monoid> v;
		v.reserve(n);
		for(const auto &e : r) {
			const auto &r1 = rolling_hash::r1;
			const auto &r2 = rolling_hash::r2;
			monoid m{.h{e % r1, e % r2}, .hr{e % r1, e % r2}, .l = 1};
			v.push_back(m);
		}

		t = decltype(t)(v);
	}

	// [l, r)
	[[nodiscard]] monoid prod(const int l, const int r) const {
		return t.prod(l, r);
	}

	// [l, r)
	[[nodiscard]] monoid rprod(const int l, const int r) const {
		return t.prod(l, r).reversed();
	}

	[[nodiscard]] monoid all_prod() const {
		return t.all_prod();
	}

	[[nodiscard]] monoid all_rprod() const {
		return t.all_prod().reversed();
	}

	void set(const int i, const ull c) {
		const auto &r1 = rolling_hash::r1;
		const auto &r2 = rolling_hash::r2;
		const monoid m{.h{c % r1, c % r2}, .hr{c % r1, c % r2}, .l = 1};
		t.set(i, m);
	}
};
