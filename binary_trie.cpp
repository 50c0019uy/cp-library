// https://github.com/50c0019uy/cp-library/blob/main/binary_trie.cpp

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

using namespace std;
using ull = unsigned long long;

template<int bit_width = 63>
class binary_trie {
	private:
	struct node {
		array<int, 2> common{0, 0};
		array<int, 2> children{null, null};
		static constexpr int null = -1;
	};

	public:
	binary_trie() : nodes(vector<node>()) {
		static_assert(1 <= bit_width && bit_width <= 8 * sizeof(ull));
		nodes.push_back(node());
	}

	[[nodiscard]] int count() const noexcept {
		return nodes[0].common[0] + nodes[0].common[1];
	}

	[[nodiscard]] int count(const ull x) const {
		assert(x <= MAX);
		return (x == MAX ? count() : count_lower(x + 1)) - count_lower(x);
	}

	[[nodiscard]] size_t size() const noexcept {
		return nodes.size();
	}

	void insert(const ull x) {
		assert(x <= MAX);
		if(nodes.capacity() < nodes.size() + bit_width) {
			nodes.reserve(std::max(nodes.size() + bit_width, nodes.capacity() * 2));
		}
		node *cur = &nodes[0];
		for(int i = bit_width - 1; i >= 0; i--) {
			const int b = (x ^ lazy) >> i & 1;
			cur->common[b]++;
			if(cur->children[b] == node::null) {
				nodes.push_back(node());
				cur->children[b] = nodes.size() - 1;
			}
			cur = &nodes[cur->children[b]];
		}
		cur->common[0]++;
	}

	/**
	 * @return 削除する対象が存在したか.
	 */
	bool erase(const ull x) {
		assert(x <= MAX);
		if(! contains(x)) return false;
		node *cur = &nodes[0];
		for(int i = bit_width - 1; i >= 0; i--) {
			const int b = (x ^ lazy) >> i & 1;
			cur->common[b]--;
			cur = &nodes[cur->children[b]];
		}
		const bool ret = cur->common[0] > 0;
		if(ret) cur->common[0]--;
		return ret;
	}

	[[nodiscard]] bool contains(const ull x) const {
		assert(x <= MAX);
		return count(x) > 0;
	}

	/**
	 * @param k 1-origin
	 */
	[[nodiscard]] ull get_kth(int k) const {
		assert(1 <= k);
		assert(k <= count());
		k--;
		const node *cur = &nodes[0];
		ull res = 0;
		for(int i = bit_width - 1; i >= 0; i--) {
			const int inv = lazy >> i & 1;
			const int big = int(cur->common[inv] <= k);
			res = res * 2 + big;
			if(big) k -= cur->common[inv];
			cur = &nodes[cur->children[inv ^ big]];
		}
		return res;
	}

	[[nodiscard]] ull min() const {
		assert(count() > 0);
		return get_kth(1);
	}

	[[nodiscard]] ull max() const {
		assert(count() > 0);
		return get_kth(count());
	}

	/**
	 * @brief x以上で最小の要素、存在しなければMAX.
	 */
	[[nodiscard]] ull lower_bound(const ull x) const {
		assert(x <= MAX);
		const int idx = count_lower(x);
		return idx == count() ? MAX : get_kth(idx + 1);
	}

	/**
	 * @brief x未満の要素の数.
	 */
	[[nodiscard]] int count_lower(const ull x) const {
		assert(x <= MAX);
		const node *cur = &nodes[0];
		int cnt = 0;
		for(int i = bit_width - 1; i >= 0; i--) {
			const int inv = lazy >> i & 1;
			if(x >> i & 1) {
				cnt += cur->common[inv];
			}
			const int idx = cur->children[inv ^ (x >> i & 1)];
			if(idx == node::null) break;
			cur = &nodes[idx];
		}
		return cnt;
	}

	void xor_all(const ull x) noexcept {
		lazy ^= x;
	}

	private:
	static constexpr ull MAX =
		bit_width == 8 * sizeof(ull) ? numeric_limits<ull>::max() : ~(~0ULL << bit_width);
	vector<node> nodes;
	ull lazy = 0;
};

// template class binary_trie<60>;
// template class binary_trie<63>;
