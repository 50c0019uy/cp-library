// https://github.com/50c0019uy/cp-library/blob/main/segment_graph.cpp

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <queue>
#include <ranges>
#include <utility>
#include <vector>

using namespace std;
using ll = long long;

class segment_graph {
	private:
	using pall = pair<ll, ll>;
	using graph = vector<vector<pall>>;
	static constexpr ll inf = numeric_limits<ll>::max();
	const ll n;
	const size_t n2;
	graph nei;
	vector<ll> out_t, in_t;

	void build_segment() {
		if(n == 0) return;
		nei.resize(n2 + 2 * (n2 - 1));

		in_t.resize(1 + (n2 - 1) + n2);
		out_t.resize(1 + (n2 - 1) + n2);
		for(size_t i = 1; i < 1 + (n2 - 1) + n2; i++) {
			if(i < n2) {
				out_t[i] = i + n2 - 1;
				in_t[i] = i + n2 + (n2 - 1) - 1;
			} else {
				out_t[i] = i - n2;
				in_t[i] = i - n2;
			}
		}

		for(size_t i = 1; 2 * i < out_t.size(); i++) {
			nei[out_t[2 * i]].emplace_back(out_t[i], 0);
			nei[out_t[2 * i + 1]].emplace_back(out_t[i], 0);
		}

		for(size_t i = 1; 2 * i < in_t.size(); i++) {
			nei[in_t[i]].emplace_back(in_t[2 * i], 0);
			nei[in_t[i]].emplace_back(in_t[2 * i + 1], 0);
		}
	}

	void _add_v2i(const ll v, ll l, ll r, const ll w) {
		assert(0 <= v && v < (ll)nei.size());
		assert(0 <= l);
		assert(l <= r);
		assert(r <= n);
		l += n2;
		r += n2;
		while(l < r) {
			if(l % 2) {
				nei[v].emplace_back(in_t[l], w);
				l++;
			}
			if(r % 2) {
				nei[v].emplace_back(in_t[r - 1], w);
			}
			l /= 2;
			r /= 2;
		}
	}

	void _add_i2v(ll l, ll r, const ll v, const ll w = 1) {
		assert(0 <= l);
		assert(l <= r);
		assert(r <= n);
		assert(0 <= v && v < (ll)nei.size());
		l += n2;
		r += n2;
		while(l < r) {
			if(l % 2) {
				nei[out_t[l]].emplace_back(v, w);
				l++;
			}
			if(r % 2) {
				nei[out_t[r - 1]].emplace_back(v, w);
			}
			l /= 2;
			r /= 2;
		}
	}

	public:
	segment_graph(const ll n) : segment_graph(graph(n)) {};

	segment_graph(const graph &g) : n(g.size()), n2(bit_ceil(g.size())), nei(g) {
		build_segment();
	}

	segment_graph(graph &&g) : n(g.size()), n2(bit_ceil(g.size())), nei(std::move(g)) {
		build_segment();
	}

	void add_v2v(const ll u, const ll v, const ll w = 1) {
		assert(0 <= u && u < n);
		assert(0 <= v && v < n);
		nei[u].emplace_back(v, w);
	}

	void add_v2v_bidirectional(const ll u, const ll v, const ll w = 1) {
		add_v2v(u, v, w);
		add_v2v(v, u, w);
	}

	// [l, r)
	void add_v2i(const ll v, const ll l, const ll r, const ll w = 1) {
		assert(0 <= v && v < n);
		_add_v2i(v, l, r, w);
	}

	// [l, r)
	void add_v2i_bidirectional(const ll v, const ll l, const ll r, const ll w = 1) {
		add_v2i(v, l, r, w);
		add_i2v(l, r, v, w);
	}

	// [l, r)
	void add_i2v(const ll l, const ll r, const ll v, const ll w = 1) {
		assert(0 <= v && v < n);
		_add_i2v(l, r, v, w);
	}

	// [l, r)
	void add_i2v_bidirectional(const ll l, const ll r, const ll v, const ll w = 1) {
		add_v2i_bidirectional(v, l, r, w);
	}

	// [l1, r1), [l2, r2)
	void add_i2i(const ll l1, const ll r1, const ll l2, const ll r2, const ll w = 1) {
		assert(0 <= l1);
		assert(l1 <= r1);
		assert(r1 <= n);
		assert(0 <= l2);
		assert(l2 <= r2);
		assert(r2 <= n);
		nei.emplace_back();
		const ll newv = nei.size() - 1;
		_add_i2v(l1, r1, newv, w);
		_add_v2i(newv, l2, r2, 0);
	}

	// [l1, r1), [l2, r2)
	void add_i2i_bidirectional(const ll l1, const ll r1, const ll l2, const ll r2, const ll w = 1) {
		nei.emplace_back();
		const ll newv = nei.size() - 1;
		_add_i2v(l1, r1, newv, w);
		_add_v2i(newv, l2, r2, 0);

		_add_i2v(l2, r2, newv, w);
		_add_v2i(newv, l1, r1, 0);
	}

	[[nodiscard]] const graph &get_graph() const {
		return nei;
	}

	[[nodiscard]] vector<ll> dijkstra(const ll s = 0) const {
		assert(0 <= s && s < n);
		priority_queue<pall, vector<pall>, greater<>> q;
		vector<bool> seen(nei.size());
		vector<ll> dist(nei.size(), inf);
		dist[s] = 0;
		q.emplace(0, s);
		while(q.size()) {
			const auto [cost, i] = q.top();
			q.pop();
			if(seen[i]) continue;
			if(dist[i] != cost) continue;
			seen[i] = true;
			for(const auto &[e, w] : nei[i]) {
				if(seen[e]) continue;
				if(dist[e] <= cost + w) continue;
				dist[e] = cost + w;
				q.emplace(dist[e], e);
			}
		}

		dist.resize(n);
		return dist;
	}

	[[nodiscard]] ll dijkstra(const ll s, const ll t) const {
		assert(0 <= s && s < n);
		assert(0 <= t && t < n);
		const auto d = dijkstra(s);
		return d[t];
	}

	// もし頂点が負閉路に含まれるなら、コストを -1 * numeric_limits<ll>::max()
	// とする.
	[[nodiscard]] vector<ll> bellman_ford(const ll s = 0) const {
		assert(0 <= s && s < n);
		vector<ll> dist(nei.size(), inf);
		dist[s] = 0;

		for(auto _ : views::iota(size_t(0), nei.size())) {
			for(size_t j = 0; j < nei.size(); j++) {
				if(dist[j] == inf) continue;
				for(const auto &[e, w] : nei[j]) {
					dist[e] = min(dist[e], dist[j] + w);
				}
			}
		}

		queue<ll> q;

		for(size_t i = 0; i < nei.size(); i++) {
			if(dist[i] == inf) continue;
			for(const auto &[e, w] : nei[i]) {
				if(dist[e] > dist[i] + w) q.push(e);
			}
		}

		while(q.size()) {
			const ll i = q.front();
			q.pop();
			if(dist[i] == -inf) continue;
			dist[i] = -inf;
			for(const auto &[e, _] : nei[i]) {
				if(dist[e] == -inf) continue;
				q.push(e);
			}
		}

		dist.resize(n);
		return dist;
	}

	// もし頂点が負閉路に含まれるなら、コストを -1 * numeric_limits<ll>::max()
	// とする.
	[[nodiscard]] ll bellman_ford(const ll s, const ll t) const {
		assert(0 <= s && s < n);
		assert(0 <= t && t < n);
		const auto d = bellman_ford(s);
		return d[t];
	}
};
