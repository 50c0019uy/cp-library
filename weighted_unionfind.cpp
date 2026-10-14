#include <cassert>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <vector>

// yosupo library checker verified.
// https://judge.yosupo.jp/submission/239508

using namespace std;

template<class difftype>
class weighted_unionfind {
	public:
	weighted_unionfind(const int n) : n(n) {
		parent.resize(n);
		iota(parent.begin(), parent.end(), 0);
		siz.resize(n, 1);
		weight.resize(n);
	}

	int leader(const int x) {
		assert(0 <= x && x < n);
		if(parent[x] == x) return x;
		const int par = leader(parent[x]);
		weight[x] += weight[parent[x]];
		return parent[x] = par;
	}

	[[nodiscard]] size_t size(const int x) {
		assert(0 <= x && x < n);
		return siz[leader(x)];
	}

	[[nodiscard]] bool same(const int x, const int y) {
		assert(0 <= x && x < n);
		assert(0 <= y && y < n);
		return leader(x) == leader(y);
	}

	// y - x
	[[nodiscard]] difftype diff(const int x, const int y) {
		assert(0 <= x && x < n);
		assert(0 <= y && y < n);
		if(! same(x, y)) [[unlikely]]
			throw invalid_argument("x and y must be same");
		leader(x), leader(y);
		return weight[y] - weight[x];
	}

	// x + w = y
	int merge(const int x, const int y, const difftype w) {
		assert(0 <= x && x < n);
		assert(0 <= y && y < n);
		const int RX = leader(x);
		const int RY = leader(y);
		if(RX == RY) return RX;

		if(siz[RX] < siz[RY]) {
			parent[RX] = RY;
			weight[RX] = weight[y] - weight[x] - w;
			siz[RY] = siz[RX] + siz[RY];
		} else {
			parent[RY] = RX;
			weight[RY] = weight[x] - weight[y] + w;
			siz[RX] = siz[RX] + siz[RY];
		}
		return parent[RX];
	}

	private:
	const int n;
	vector<int> parent;
	vector<int> siz;
	vector<difftype> weight;
};
