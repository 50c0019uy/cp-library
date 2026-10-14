#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <ranges>
#include <utility>
#include <vector>

using namespace std;

class undoable_unionfind {
	private:
	struct modify {
		int i, oldv;
	};

	const int n;
	vector<int> par_or_size;
	vector<vector<modify>> hist;

	public:
	undoable_unionfind(const int _n) : n(_n) {
		par_or_size = vector<int>(n, -1);
	}

	void query_reserve(const int q) {
		hist.reserve(q);
	}

	[[nodiscard]] int leader(int x) const {
		assert(0 <= x && x < n);
		while(par_or_size[x] >= 0) {
			x = par_or_size[x];
		}
		return x;
	}

	[[nodiscard]] int size(const int x) const {
		assert(0 <= x && x < n);
		return -par_or_size[leader(x)];
	}

	[[nodiscard]] bool same(const int x, const int y) const {
		assert(0 <= x && x < n);
		assert(0 <= y && y < n);
		return leader(x) == leader(y);
	}

	void merge(const int x, const int y) {
		assert(0 <= x && x < n);
		assert(0 <= y && y < n);
		hist.emplace_back();
		if(same(x, y)) return;
		const auto [small, large] = minmax(
			{leader(x), leader(y)}, [this](const int x, const int y) { return size(x) < size(y); });

		hist.back().push_back({.i = large, .oldv = par_or_size[large]});
		par_or_size[large] += par_or_size[small];
		hist.back().push_back({.i = small, .oldv = par_or_size[small]});
		par_or_size[small] = large;
	}

	void undo(const int n = 1) {
		assert(0 <= n && n <= (int)hist.size());
		for(auto _ : views::iota(0, n)) {
			const auto mods = std::move(hist.back());
			hist.pop_back();
			for(const auto &mod : mods) {
				par_or_size[mod.i] = mod.oldv;
			}
		}
	}
};

class offline_dynamic_connectivity {
	private:
	struct edge {
		int u, v;

		int operator<=>(const edge &e) const {
			if(u != e.u) return u - e.u;
			return v - e.v;
		}

		bool operator==(const edge &e) const = default;
	};

	struct query {
		const enum : uint8_t { add, remove, same } t;

		const edge e;
	};

	const int n;
	vector<query> queries;
	vector<edge> edges;

	[[nodiscard]] vector<vector<int>> calc_live_ranges() const {
		assert(std::ranges::is_sorted(edges));
		const int q = queries.size();
		vector<vector<int>> lrs(edges.size());
		for(const int i : views::iota(0, q)) {
			const auto &[t, e] = queries[i];
			const auto it = std::ranges::lower_bound(edges, e);
			if(it == edges.end() || *it != e) continue;
			const auto idx = it - edges.begin();
			switch(t) {
				case query::same: break;
				case query::add:
					if(lrs[idx].size() % 2 == 0) lrs[idx].push_back(i);
					break;

				case query::remove:
					if(lrs[idx].size() % 2 == 1) lrs[idx].push_back(i);
					break;
			}
		}
		return lrs;
	}

	public:
	offline_dynamic_connectivity(const int _n) : n(_n) {}

	void query_reserve(const int q) {
		queries.reserve(q);
		edges.reserve(q);
	}

	void add_query(const int u, const int v) {
		assert(0 <= u && u < n);
		assert(0 <= v && v < n);
		const edge e{.u = u, .v = v};
		queries.push_back({.t = query::add, .e = e});
		edges.push_back(e);
	}

	void remove_query(const int u, const int v) {
		assert(0 <= u && u < n);
		assert(0 <= v && v < n);
		queries.push_back({.t = query::remove, .e = {.u = u, .v = v}});
	}

	void same_query(const int u, const int v) {
		assert(0 <= u && u < n);
		assert(0 <= v && v < n);
		queries.push_back({.t = query::same, .e = {.u = u, .v = v}});
	}

	[[nodiscard]] vector<int> solve() {
		const int q = queries.size();
		std::ranges::sort(edges);
		auto lrs = calc_live_ranges();

		for(auto &lr : lrs) {
			if(lr.size() % 2 == 1) lr.push_back(q);
		}

		const int tsize = bit_ceil((unsigned)q) - 1;
		vector<vector<edge>> t(1 + tsize + q);
		for(const int i : views::iota(0, (int)edges.size())) {
			const auto &e = edges[i];
			const auto &lr = lrs[i];
			for(int j = 0; j < (int)lr.size(); j += 2) {
				int l = lr[j] + tsize + 1;
				int r = lr[j + 1] + tsize + 1;
				while(l < r) {
					if(l & 1) t[l].push_back(e), l++;
					if(r & 1) t[r - 1].push_back(e);
					l >>= 1;
					r >>= 1;
				}
			}
		}

		vector<int> ans;
		undoable_unionfind uf(n);
		const auto dfs = [this, &t, tsize, &uf, &ans](auto &&self, const int i) -> void {
			if(i >= t.size()) return;
			for(const auto &[u, v] : t[i]) {
				uf.merge(u, v);
			}
			self(self, i * 2);
			self(self, i * 2 + 1);
			if(i > tsize && queries[i - tsize - 1].t == query::same) {
				const auto &[u, v] = queries[i - tsize - 1].e;
				ans.push_back(uf.same(u, v));
			}
			uf.undo(t[i].size());
		};
		dfs(dfs, 1);
		return ans;
	}
};

using offline_dyconne = offline_dynamic_connectivity;
