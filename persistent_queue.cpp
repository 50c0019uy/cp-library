// https://github.com/50c0019uy/cp-library/blob/main/persistent_queue.cpp

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std;

// yosupo library checker verified.
// https://judge.yosupo.jp/submission/305481
// ダブリング式.
template<class T, size_t maxn = 1'500'000>
class persistent_queue {
	public:
	static_assert(is_default_constructible_v<T>, "T must be default constructible");

	struct Node {
		const T val{};
		const array<size_t, max(1, bit_width(maxn))> nexts{};
	};

	private:
	static vector<Node> nodes;

	size_t node;
	size_t count;

	persistent_queue(const size_t _node, const size_t _count) : node(_node), count(_count) {}

	public:
	static void reserve(const size_t n) {
		nodes.reserve(n);
	}

	persistent_queue() : node(0), count(0) {}

	[[nodiscard]] size_t size() const {
		return count;
	}

	[[nodiscard]] bool empty() const {
		return size() == 0;
	}

	[[nodiscard]] const T &top() const {
		assert(not empty());
		size_t r = count - 1;
		size_t cur = node;
		while(r) {
			cur = nodes[cur].nexts[bit_width(r) - 1];
			r ^= bit_floor(r);
		}
		return nodes[cur].val;
	}

	[[nodiscard]] persistent_queue push(const T &val) const {
		return push(T(val));
	}

	[[nodiscard]] persistent_queue push(T &&val) const {
		remove_const_t<decltype(Node::nexts)> nexts;
		size_t cur = node;
		for(size_t i = 0; i < nexts.size(); i++) {
			nexts[i] = cur;
			cur = nodes[cur].nexts[i];
		}
		nodes.emplace_back(std::move(val), nexts);
		return persistent_queue(nodes.size() - 1, count + 1);
	}

	[[nodiscard]] persistent_queue pop(const size_t n = 1) const {
		assert(n <= size());
		return persistent_queue(node, count - n);
	}
};

template<class T, size_t maxn>
vector<typename persistent_queue<T, maxn>::Node> persistent_queue<T, maxn>::nodes(1);
