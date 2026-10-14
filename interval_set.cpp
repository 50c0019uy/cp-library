// https://github.com/50c0019uy/cp-library/blob/main/interval_set.cpp

#include <cassert>
#include <concepts>
#include <iterator>
#include <limits>
#include <set>
#include <utility>

using namespace std;
using ll = long long;
using pall = pair<ll, ll>;

// verified
// https://atcoder.jp/contests/past202104-open/submissions/71617207

template<default_initializable T>
struct interval {
	ll l{}, r{};
	T v{};

	bool operator<(const interval &e) const {
		if(l != e.l) return l < e.l;
		return r < e.r;
	}
};

// 半開区間.
template<copy_constructible T = int,
	class addProc = decltype([](interval<T>) {}),
	class delProc = decltype([](interval<T>) {})>
requires invocable<addProc, interval<T>> && invocable<delProc, interval<T>>
class interval_set {
	private:
	using interval_t = interval<T>;
	static constexpr ll inf = numeric_limits<ll>::max();

	[[no_unique_address]] addProc add_proc;
	[[no_unique_address]] delProc del_proc;
	set<interval_t> s;

	public:
	interval_set(T /*unused*/ = 0, const addProc &_add = addProc(), const delProc &_del = delProc())
		: add_proc(_add), del_proc(_del) {
		s.emplace(-inf, -inf);
		s.emplace(inf, inf);
	}

	void add(ll l, ll r, const T &val = {}) {
		assert(l <= r);
		del(l, r);

		if constexpr(equality_comparable<T>) {
			if(const auto it = prev(s.lower_bound({l, l})); it->r == l && it->v == val) {
				l = it->l;
				del_proc(*it);
				s.erase(it);
			}
			if(const auto it = s.lower_bound({r, r}); it->l == r && it->v == val) {
				r = it->r;
				del_proc(*it);
				s.erase(it);
			}
		}

		add_proc(interval_t{l, r, val});
		s.insert({l, r, val});
	}

	void add(const pall &p, const T &val = {}) {
		add(p.first, p.second, val);
	}

	void del(const ll l, const ll r) {
		assert(l <= r);
		if(const auto it = prev(s.lower_bound({l, l})); it->l <= l && l < it->r) {
			if(const interval_t nlval{it->l, l, it->v}; nlval.l < nlval.r) {
				add_proc(nlval);
				s.insert(nlval);
			}
			if(const interval_t nrval{r, it->r, it->v}; nrval.l < nrval.r) {
				add_proc(nrval);
				s.insert(nrval);
			}
			del_proc(*it);
			s.erase(it);
		}

		for(auto it = s.lower_bound({l, l}); l <= it->l && it->l < r; it = s.erase(it)) {
			del_proc(*it);
			if(r < it->r) {
				const interval_t nval{r, it->r, it->v};
				add_proc(nval);
				s.insert(nval);
			}
		}
	}

	void del(const pall &p) {
		del(p.first, p.second);
	}

	auto cross(const ll l, const ll r) const {
		assert(l <= r);
		auto lit = s.lower_bound({l, l});
		if(prev(lit)->l <= l && l < prev(lit)->r) lit--;
		const auto rit = s.lower_bound({r, r});
		return pair{as_const(lit), rit};
	}

	auto cross(const pall &p) const {
		return cross(p.first, p.second);
	}
};
