// https://github.com/50c0019uy/cp-library/blob/main/potential_unionfind.hpp

#include <cassert>
#include <cstddef>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <vector>

using namespace std;
using ll = long long;

template<class T,
	auto composition = std::plus<T>{},
	auto inverse = std::negate<T>{},
	auto e = [] { return T{}; },
	bool Rollback = false>
requires requires (T x, T y) {
	{ composition(x, y) } -> convertible_to<T>;
	{ inverse(x) } -> convertible_to<T>;
	{ e() } -> convertible_to<T>;
}
class potential_unionfind {
	private:
	const ll n;
	vector<ll> par_size;
	vector<T> pot, pot_rev;
	vector<vector<std::pair<ll, ll>>> par_hist;
	vector<vector<std::pair<ll, T>>> pot_hist, pot_rev_hist;

	[[nodiscard]] ll _leader(const ll x) const {
		if(par_size[x] < 0) return x;

		return _leader(par_size[x]);
	}

	[[nodiscard]] T leader_pot(const ll x) const {
		if(par_size[x] < 0) return e();

		return composition(leader_pot(par_size[x]), pot[x]);
	}

	[[nodiscard]] T leader_pot_rev(const ll x) const {
		if(par_size[x] < 0) return e();

		return composition(pot_rev[x], leader_pot_rev(par_size[x]));
	}

	public:
	potential_unionfind(const ll n_) : n(n_), par_size(n_, -1), pot(n_, e()), pot_rev(n_, e()) {}

	[[nodiscard]] ll leader(const ll x) const {
		assert(0 <= x && x < n);
		if(par_size[x] < 0) return x;

		return _leader(par_size[x]);
	}

	[[nodiscard]] bool same(const ll x, const ll y) const {
		assert(0 <= x && x < n);
		assert(0 <= y && y < n);

		return leader(x) == leader(y);
	}

	[[nodiscard]] ll size(const ll x) const {
		assert(0 <= x && x < n);

		return -par_size[leader(x)];
	}

	[[nodiscard]] T diff(const ll x, const ll y) const {
		assert(0 <= x && x < n);
		assert(0 <= y && y < n);

		assert(same(x, y));

		return composition(leader_pot_rev(y), leader_pot(x));
	}

	void merge(const ll x, const ll y, const T &wxy) {
		merge(x, y, wxy, inverse(wxy));
	}

	void merge(ll x, ll y, T wxy, T wyx) {
		assert(0 <= x && x < n);
		assert(0 <= y && y < n);

		ll lx = leader(x);
		ll ly = leader(y);

		if(lx == ly) return;

		if(-par_size[lx] < -par_size[ly]) {
			swap(x, y);
			swap(lx, ly);
			swap(wxy, wyx);
		}

		const T wlxly =
			composition(composition(inverse(leader_pot_rev(y)), wxy), inverse(leader_pot(x)));

		const T wlylx =
			composition(composition(inverse(leader_pot_rev(x)), wyx), inverse(leader_pot(y)));

		if constexpr(Rollback) {
			pot_hist.emplace_back();
			pot_rev_hist.emplace_back();
			par_hist.emplace_back();

			pot_hist.back().emplace_back(ly, pot[ly]);
			pot_rev_hist.back().emplace_back(ly, pot_rev[ly]);
			par_hist.back().emplace_back(lx, par_size[lx]);
			par_hist.back().emplace_back(ly, par_size[ly]);
		}

		pot[ly] = wlylx;
		pot_rev[ly] = wlxly;
		par_size[lx] += par_size[ly];
		par_size[ly] = lx;
	}

	void rollback()
	requires Rollback
	{
		assert(pot_hist.size());

		for(const auto &[i, val] : pot_hist.back()) pot[i] = val;
		for(const auto &[i, val] : pot_rev_hist.back()) pot_rev[i] = val;
		for(const auto &[i, val] : par_hist.back()) par_size[i] = val;

		pot_hist.pop_back();
		pot_rev_hist.pop_back();
		par_hist.pop_back();
	}
};
