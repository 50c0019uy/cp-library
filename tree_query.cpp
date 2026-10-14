// https://github.com/50c0019uy/cp-library/blob/main/tree_query.cpp

#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <ranges>
#include <utility>
#include <variant>
#include <vector>

#include "atcoder/segtree.hpp"

using namespace std;
using namespace atcoder;

using ll = long long;

template<class T = ll, auto op = [](T a, T b) { return a + b; }, auto el = [] { return 0LL; }>
requires requires (T a, T b) {
	{ op(a, b) } -> convertible_to<T>;
	{ el() } -> convertible_to<T>;
}
class tree_static_monoid {
	private:
	struct edge {
		int u, v;
		T w;
	};

	vector<edge> edges;

	bool built = false;
	size_t n;
	vector<vector<pair<ll, T>>> nei;
	vector<vector<int>> dp_ancestor;
	vector<vector<T>> dp_prod_td;
	vector<vector<T>> dp_prod_bu;
	vector<T> subtree_prod;
	vector<int> depth;
	vector<bool> seen;

	T dfs(const ll i, const ll d) {
		seen[i] = true;
		depth[i] = d;
		T sum = el();
		for(const auto &[e, w] : nei[i]) {
			if(seen[e]) continue;
			dp_ancestor[0][e] = i;
			dp_prod_bu[0][e] = w;
			dp_prod_td[0][e] = w;
			sum = op(sum, w);
			sum = op(sum, dfs(e, d + 1));
		}
		return subtree_prod[i] = sum;
	}

	public:
	tree_static_monoid() = default;

	void add_edge(ll u, ll v, const T &w = {}) {
		edges.emplace_back(u, v, w);
		edges.emplace_back(v, u, w);
	}

	void build(const ll root) {
		assert(not built);
		built = true;
		n = 1 + (edges.empty() ? 0 : std::ranges::max(edges | views::transform([](const edge &e) {
			return max(e.u, e.v);
		})));
		nei.resize(n);
		for(const auto &[u, v, w] : edges) {
			nei[u].emplace_back(v, w);
			nei[v].emplace_back(u, w);
		}
		depth.resize(n);
		seen.resize(n);
		dp_ancestor.resize(bit_width(n), vector<int>(n));
		dp_ancestor[0][root] = root;
		dp_prod_td.resize(bit_width(n), vector<T>(n, el()));
		dp_prod_bu.resize(bit_width(n), vector<T>(n, el()));
		subtree_prod.resize(n, el());
		dfs(root, 0);
		for(size_t i = 1; i < bit_width(n); i++) {
			for(size_t j = 0; j < n; j++) {
				const int mid_ancestor = dp_ancestor[i - 1][j];
				dp_ancestor[i][j] = dp_ancestor[i - 1][mid_ancestor];
				dp_prod_bu[i][j] = op(dp_prod_bu[i - 1][j], dp_prod_bu[i - 1][mid_ancestor]);
				dp_prod_td[i][j] = op(dp_prod_td[i - 1][mid_ancestor], dp_prod_td[i - 1][j]);
			}
		}
	}

	void traverse(const ll root) {
		build(root);
	}

	[[nodiscard]] ll lca(ll x, ll y) const {
		assert(0 <= x && x < n);
		assert(0 <= y && y < n);
		if(depth[x] < depth[y]) swap(x, y);

		for(uint32_t dist = depth[x] - depth[y]; dist;) {
			const auto i = countr_zero(dist);
			x = dp_ancestor[i][x];
			dist ^= 1U << i;
		}
		if(x == y) return x;
		for(const size_t i : views::iota(0, bit_width(n)) | views::reverse) {
			if(dp_ancestor[i][x] != dp_ancestor[i][y]) {
				x = dp_ancestor[i][x];
				y = dp_ancestor[i][y];
			}
		}
		return dp_ancestor[0][x];
	}

	[[nodiscard]] T path(ll x, ll y) const {
		assert(0 <= x && x < n);
		assert(0 <= y && y < n);

		const ll lcaxy = lca(x, y);
		T lp = el();
		for(uint32_t dist = depth[x] - depth[lcaxy]; dist;) {
			const auto i = countr_zero(dist);
			lp = op(lp, dp_prod_bu[i][x]);
			x = dp_ancestor[i][x];
			dist ^= 1U << i;
		}
		T rp = el();
		for(uint32_t dist = depth[y] - depth[lcaxy]; dist;) {
			const auto i = countr_zero(dist);
			rp = op(dp_prod_td[i][y], rp);
			y = dp_ancestor[i][y];
			dist ^= 1U << i;
		}
		return op(lp, rp);
	}

	[[nodiscard]] T subtree(const ll x) const {
		assert(0 <= x && x < n);
		return subtree_prod[x];
	}
};

template<class T = monostate>
class euler_tour {
	struct edge {
		int u, v;
		T w;
	};

	vector<edge> edges;

	private:
	void dfs(const ll i, const ll d) {
		seen[i] = true;
		depth.push_back(d);
		in[i] = edge_tour.size();
		edge_tour.emplace_back(preorder, i);
		for(const auto &[e, _] : nei[i]) {
			if(seen[e]) continue;
			parent[e] = i;
			dfs(e, d + 1);
			depth.push_back(d);
		}
		out[i] = edge_tour.size();
		edge_tour.emplace_back(postorder, i);
	}

	public:
	bool traversed = false;
	size_t n;
	vector<vector<pair<ll, T>>> nei;
	vector<int> in, out, parent;

	enum order : uint8_t { preorder, postorder };

	vector<pair<order, int>> edge_tour;

	vector<int> depth;
	vector<bool> seen;

	euler_tour() = default;

	void add_edge(ll u, ll v, const T &w = {}) {
		edges.emplace_back(u, v, w);
		edges.emplace_back(v, u, w);
	}

	void traverse(const ll root) {
		assert(not traversed);
		traversed = true;
		n = 1 + (edges.empty() ? 0 : std::ranges::max(edges | views::transform([](const edge &e) {
			return max(e.u, e.v);
		})));
		nei.resize(n);
		for(const auto &[u, v, w] : edges) {
			nei[u].emplace_back(v, w);
			nei[v].emplace_back(u, w);
		}
		for(auto &v : nei) {
			std::ranges::sort(v, {}, &pair<ll, T>::first);
		}
		edge_tour.reserve(2 * n);
		depth.reserve(2 * n - 1);
		in.resize(n);
		out.resize(n);
		parent.resize(n);
		seen.resize(n);
		dfs(root, 0);
	}
};

template<class T = ll,
	auto op = [](T a, T b) { return a + b; },
	auto el = [] { return 0LL; },
	auto inv = [](ll a) { return -a; }>
requires requires (T a, T b) {
	{ op(a, b) } -> convertible_to<T>;
	{ el() } -> convertible_to<T>;
	{ inv(a) } -> convertible_to<T>;
}
class et_static_group : public euler_tour<T> {
	using order = euler_tour<T>::order;

	class range_minidx {
		public:
		range_minidx() = default;

		range_minidx(vector<int> &&_v) : n(_v.size()), lb_n(bit_width(n)), v(std::move(_v)) {
			tab = vector(lb_n, vector<int>(n));
			iota(tab[0].begin(), tab[0].end(), 0);
			for(size_t j = 1; j < lb_n; j++) {
				for(size_t i = 0; i + (1LL << (j - 1)) < n; i++) {
					const ll i1 = tab[j - 1][i];
					const ll i2 = tab[j - 1][i + (1LL << (j - 1))];
					tab[j][i] = v[i1] > v[i2] ? i2 : i1;
				}
			}
		}

		// [L, R)
		[[nodiscard]] ll query(const ll l, const ll r) const {
			assert(0 <= l);
			assert(l < r);
			assert(r <= (ll)n);
			const size_t lb_w = bit_width<size_t>(r - l) - 1;
			const ll i1 = tab[lb_w][l];
			const ll i2 = tab[lb_w][r - (1LL << lb_w)];
			return v[i1] > v[i2] ? i2 : i1;
		}

		private:
		size_t n = 0;
		size_t lb_n;
		vector<int> v;
		vector<vector<int>> tab;
	};

	public:
	et_static_group() = default;

	[[nodiscard]] ll lca(const ll x, const ll y) const {
		assert(0 <= x && x < this->n);
		assert(0 <= y && y < this->n);
		const auto [l, r] = minmax(this->in[x], this->in[y]);
		const ll idx = this->depth_t.query(l, r + 1);
		auto [order, e] = this->edge_tour[idx];
		if(order == order::postorder) {
			e = this->parent[e];
		}
		return e;
	}

	[[nodiscard]] T path(const ll x, const ll y) const {
		assert(0 <= x && x < this->n);
		assert(0 <= y && y < this->n);
		const ll xidx = this->in[x];
		const ll yidx = this->in[y];
		const ll lca_idx = this->in[lca(x, y)];
		T x2lca = op(edge_val_rprod[xidx], inv(edge_val_rprod[lca_idx]));
		T lca2y = op(inv(edge_val_prod[lca_idx]), edge_val_prod[yidx]);
		return op(x2lca, lca2y);
	}

	[[nodiscard]] T subtree(const ll x) const {
		assert(0 <= x && x < this->n);
		const ll l = this->in[x];
		const ll r = this->out[x];
		return op(inv(edge_val_prod_noinv[l]), edge_val_prod_noinv[r]);
	}

	void traverse(const ll root) {
		euler_tour<T>::traverse(root);
		depth_t = decltype(depth_t)(std::move(this->depth));
		edge_val_prod.reserve(2 * this->n);
		edge_val_rprod.reserve(2 * this->n);
		edge_val_prod_noinv.reserve(2 * this->n);
		for(const auto &[order, e] : this->edge_tour) {
			if(e == root) {
				if(edge_val_prod.empty()) {
					edge_val_prod.push_back(el());
					edge_val_rprod.push_back(el());
					edge_val_prod_noinv.push_back(el());
				} else {
					edge_val_prod.push_back(edge_val_prod.back());
					edge_val_rprod.push_back(edge_val_rprod.back());
					edge_val_prod_noinv.push_back(edge_val_prod_noinv.back());
				}
				continue;
			}
			const auto it =
				std::ranges::lower_bound(this->nei[e], this->parent[e], {}, &pair<ll, T>::first);
			assert(it->first == this->parent[e]);
			T val = order == order::postorder ? inv(it->second) : it->second;
			edge_val_prod.push_back(op(edge_val_prod.back(), val));
			edge_val_rprod.push_back(op(val, edge_val_rprod.back()));
			edge_val_prod_noinv.push_back(order == order::preorder ? op(edge_val_prod_noinv.back(), val)
																														 : edge_val_prod_noinv.back());
		}
	}

	range_minidx depth_t;
	vector<T> edge_val_prod, edge_val_rprod, edge_val_prod_noinv;
};

template<class T = ll,
	auto op = [](T a, T b) { return a + b; },
	auto el = [] { return 0LL; },
	auto inv = [](T x) { return -x; }>
requires requires (T a, T b) {
	{ op(a, b) } -> convertible_to<T>;
	{ el() } -> convertible_to<T>;
	{ inv(a) } -> convertible_to<T>;
}
class et_dynamic_group : public euler_tour<T> {
	using order = euler_tour<T>::order;

	class range_minidx {
		public:
		range_minidx() = default;

		range_minidx(vector<int> &&_v) : n(_v.size()), lb_n(bit_width(n)), v(std::move(_v)) {
			tab = vector(lb_n, vector<int>(n));
			iota(tab[0].begin(), tab[0].end(), 0);
			for(size_t j = 1; j < lb_n; j++) {
				for(size_t i = 0; i + (1LL << (j - 1)) < n; i++) {
					const ll i1 = tab[j - 1][i];
					const ll i2 = tab[j - 1][i + (1LL << (j - 1))];
					tab[j][i] = v[i1] > v[i2] ? i2 : i1;
				}
			}
		}

		// [L, R)
		[[nodiscard]] ll query(const ll l, const ll r) const {
			assert(0 <= l);
			assert(l < r);
			assert(r <= (ll)n);
			const size_t lb_w = bit_width<size_t>(r - l) - 1;
			const ll i1 = tab[lb_w][l];
			const ll i2 = tab[lb_w][r - (1LL << lb_w)];
			return v[i1] > v[i2] ? i2 : i1;
		}

		private:
		size_t n = 0;
		size_t lb_n;
		vector<int> v;
		vector<vector<int>> tab;
	};

	public:
	[[nodiscard]] ll lca(const ll x, const ll y) const {
		assert(0 <= x && x < this->n);
		assert(0 <= y && y < this->n);
		const auto [l, r] = minmax(this->in[x], this->in[y]);
		const ll idx = this->depth_t.query(l, r + 1);
		auto [order, e] = this->edge_tour[idx];
		if(order == order::postorder) {
			e = this->parent[e];
		}
		return e;
	}

	[[nodiscard]] T path(const ll x, const ll y) const {
		assert(0 <= x && x < this->n);
		assert(0 <= y && y < this->n);
		const ll xidx = this->in[x];
		const ll yidx = this->in[y];
		const ll lca_idx = this->in[lca(x, y)];
		T x2lca = t_rev.prod(lca_idx + 1, xidx + 1);
		T lca2y = t.prod(lca_idx + 1, yidx + 1);
		return op(x2lca, lca2y);
	}

	[[nodiscard]] T subtree(const ll x) const {
		assert(0 <= x && x < this->n);
		const ll l = this->in[x] + 1;
		const ll r = this->out[x];
		return t_noinv.prod(l, r);
	}

	void set(ll u, ll v, const T &val) {
		assert(0 <= u && u < this->n);
		assert(0 <= v && v < this->n);
		if(this->depth[u] > this->depth[v]) swap(u, v);
		t.set(this->in[v], val);
		t_rev.set(this->in[v], val);
		t.set(this->out[v], inv(val));
		t_rev.set(this->out[v], inv(val));
	}

	void traverse(const ll root) {
		euler_tour<T>::traverse(root);
		depth_t = range_minidx(std::move(this->depth));
		vector<T> v(this->edge_tour.size(), el());
		vector<T> v_noinv(this->edge_tour.size(), el());
		for(size_t i = 0; i < this->edge_tour.size(); i++) {
			const auto &[order, e] = this->edge_tour[i];
			if(e == root) continue;
			const auto it =
				std::ranges::lower_bound(this->nei[e], this->parent[e], {}, &pair<ll, T>::first);
			assert(it->first == this->parent[e]);
			const T &val = it->second;
			if(order == order::preorder) {
				v[i] = val;
				v_noinv[i] = val;
			} else {
				v[i] = inv(val);
			}
		}
		t = decltype(t)(v);
		t_rev = decltype(t_rev)(v);
		t_noinv = decltype(t_noinv)(v_noinv);
	}

	range_minidx depth_t;
	segtree<T, op, el> t, t_noinv;
	segtree<T, [](T a, T b) { return op(b, a); }, el> t_rev;
};
