#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <functional>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <vector>

using namespace std;
using ll = long long;

template<class T, class comp = less<T>>
class pairing_heap {
	static_assert(requires () { requires swappable<T>; }, "T must be swappable");

	private:
	struct node {
		T val;
		uint32_t child = null;
		uint32_t sibling = null;
		static constexpr uint32_t null = 0;
	};

	static comp c;
	static vector<node> nodes;

	uint32_t root = node::null;
	ll siz = 0;

	static uint32_t _meld(const uint32_t a, const uint32_t b) {
		if(a == b) return a;
		if(a == node::null) return b;
		if(b == node::null) return a;

		const auto &[chi, par] = std::ranges::minmax(a, b, c, [](uint32_t i) { return nodes[i].val; });
		nodes[chi].sibling = nodes[par].child;
		nodes[par].child = chi;
		return par;
	}

	pairing_heap(const T &v) : root(nodes.size()), siz(1) {
		nodes.emplace_back(v);
	}

	pairing_heap(T &&v) : root(nodes.size()), siz(1) {
		nodes.emplace_back(std::move(v));
	}

	public:
	pairing_heap() = default;

	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, T>
	pairing_heap(R &&r) {
		if constexpr(std::ranges::sized_range<R>) {
			nodes.reserve(nodes.size() + std::ranges::size(r));
		}

		for(auto &&e : r) {
			if constexpr (is_rvalue_reference_v<R &&>) {
				push(std::move(e));
			} else {
				push(std::forward<decltype(e)>(e));
			}
		}
	}

	pairing_heap(const pairing_heap &) = delete;
	pairing_heap(pairing_heap &&) = default;
	pairing_heap &operator=(const pairing_heap &) = delete;
	pairing_heap &operator=(pairing_heap &&) = default;
	~pairing_heap() = default;

	[[nodiscard]] ll size() const {
		return siz;
	}

	[[nodiscard]] bool empty() const {
		return size() == 0;
	}

	[[nodiscard]] const T &top() const {
		assert(not empty());
		return nodes[root].val;
	}

	void push(const T &v) {
		*this = meld(std::move(*this), pairing_heap(v));
	}

	void push(T &&v) {
		*this = meld(std::move(*this), pairing_heap(std::move(v)));
	}

	void pop() {
		assert(not empty());
		siz--;
		if(nodes[root].child == node::null) {
			root = node::null;
			return;
		}

		uint32_t t = nodes[root].child;
		root = node::null;
		while(t != node::null) {
			const uint32_t nt = nodes[t].sibling;
			const uint32_t nnt = nodes[nt].sibling;
			root = _meld(_meld(t, nt), root);
			t = nnt;
		}
	}

	static pairing_heap meld(pairing_heap h, pairing_heap h2) {
		h.root = _meld(h.root, h2.root);
		h.siz += h2.siz;
		return h;
	}

	void reserve(const ll n) {
		if(n < siz) return;
		nodes.reserve(nodes.size() + (n - siz));
	}
};

template<class T, class comp>
comp pairing_heap<T, comp>::c{};
template<class T, class comp>
vector<typename pairing_heap<T, comp>::node> pairing_heap<T, comp>::nodes{1};
