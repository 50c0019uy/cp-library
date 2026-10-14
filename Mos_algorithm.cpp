// https://github.com/50c0019uy/cp-library/blob/main/Mos_algorithm.cpp

#include <algorithm>
#include <cmath>
#include <concepts>
#include <functional>
#include <numbers>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std;
using ll = long long;
using pall = pair<ll, ll>;

template<class T, regular_invocable<T> Proj>
requires requires (Proj proj, T v) {
	{ proj(v) } -> convertible_to<pall>;
}
class Mo {
	private:
	[[no_unique_address]] Proj proj;
	vector<pair<ll, T>> queries;

	public:
	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, T>
	Mo(R &&r, auto &&proj_) : proj(std::forward<decltype(proj_)>(proj_)) {
		for(auto &&e : std::forward<R>(r)) {
			add_query(e);
		}
	}

	void add_query(const T &v) {
		queries.emplace_back(queries.size(), v);
	}

	void add_query(T &&v) {
		queries.emplace_back(queries.size(), std::move(v));
	}

	void run(invocable auto &&add_left,
		invocable auto &&del_left,
		invocable auto &&add_right,
		invocable auto &&del_right,
		invocable<ll, T> auto &&proc_query) {
		const ll n = std::ranges::max(queries | views::transform([&](const auto &p) {
			return std::invoke(proj, p.second).second;
		}));
		const ll q = queries.size();

		const auto cmp = [B = max(1LL, static_cast<ll>(round(numbers::sqrt3 * n / sqrt(2 * q))))](
											 const pall &p1, const pall &p2) {
			const auto &[l1, r1] = p1;
			const auto &[l2, r2] = p2;

			const ll b1 = l1 / B;
			const ll b2 = l2 / B;
			if(b1 != b2) return b1 < b2;

			if(b1 % 2 == 0) {
				return r1 < r2;
			} else {
				return r1 > r2;
			}
		};

		std::ranges::sort(queries, cmp, [&](const auto &p) { return std::invoke(proj, p.second); });

		ll l = 0;
		ll r = 0;
		for(const auto &p : queries) {
			const auto &[i, e] = p;
			const auto &[ql, qr] = std::invoke(proj, e);

			while(ql < l) add_left(), l--;
			while(r < qr) add_right(), r++;
			while(l < ql) del_left(), l++;
			while(qr < r) del_right(), r--;

			proc_query(i, e);
		}
	}
};

template<std::ranges::input_range R, class Proj>
Mo(R, Proj) -> Mo<std::ranges::range_value_t<R>, std::remove_cvref_t<Proj>>;
