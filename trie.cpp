// https://github.com/50c0019uy/cp-library/blob/main/trie.cpp

#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <ranges>
#include <utility>
#include <vector>

using namespace std;

using ll = long long;

template<class T, T minv = numeric_limits<T>::min(), T maxv = numeric_limits<T>::max()>
class DenseTrie {
	public:
	struct SearchResult {
		ll common, endcnt;
	};

	private:
	struct Node {
		static constexpr uint32_t null = numeric_limits<uint32_t>::max();
		ll common = 0;
		ll endcnt = 0;
		array<uint32_t, maxv - minv + 1> cs{};
	};

	vector<Node> pool;
	uint32_t root;

	[[nodiscard]] uint32_t create_node() {
		const auto id = pool.size();
		pool.emplace_back();
		pool[id].cs.fill(Node::null);
		return id;
	}

	[[nodiscard]] static SearchResult Node2Result(const Node &node) {
		return {.common = node.common, .endcnt = node.endcnt};
	}

	public:
	DenseTrie() : root(create_node()) {}

	template<std::ranges::input_range R, class RV = std::ranges::range_value_t<R>>
	requires std::ranges::input_range<RV> && convertible_to<std::ranges::range_value_t<RV>, T>
	DenseTrie(R &&r) : root(create_node()) {
		if constexpr(std::ranges::forward_range<R> && std::ranges::sized_range<RV>) {
			size_t maxsiz = 0;
			for(auto &&s : r) {
				maxsiz += std::ranges::size(s);
			}

			reserve(maxsiz);
		}

		auto v = views::all(std::forward<R>(r));

		for(auto &&s : v) {
			add(s);
		}
	}

	[[nodiscard]] size_t size() const {
		return pool[root].common;
	}

	void reserve(const size_t n) {
		pool.reserve(n);
	}

	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, T>
	void add(R &&r, const ll n = 1) {
		if constexpr(std::ranges::sized_range<R>) {
			reserve(pool.size() + std::ranges::size(r));
		}

		auto v = views::all(std::forward<R>(r));
		uint32_t node = root;

		pool[node].common += n;
		for(const T &c : v) {
			assert(minv <= c && c <= maxv);

			if(const auto child = pool[node].cs[c - minv]; child == Node::null) {
				node = pool[node].cs[c - minv] = create_node();
			} else {
				node = child;
			}
			pool[node].common += n;
		}
		pool[node].endcnt += n;
	}

	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, T>
	void del(R &&r, const ll n = 1) {
		add(std::forward<R>(r), -n);
	}

	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, T>
	[[nodiscard]] SearchResult search(R &&r) const {
		auto v = views::all(std::forward<R>(r));
		uint32_t node = root;

		for(const T &c : v) {
			assert(minv <= c && c <= maxv);

			auto &child = pool[node].cs[c - minv];
			if(child == Node::null) {
				return {0, 0};
			}
			node = child;
		}

		return Node2Result(pool[node]);
	}
};

template<class T>
class SparseTrie {
	public:
	struct SearchResult {
		ll common, endcnt;
	};

	private:
	struct Node {
		static constexpr uint32_t null = numeric_limits<uint32_t>::max();
		ll common = 0;
		ll endcnt = 0;
		map<T, uint32_t> cs;
	};

	vector<Node> pool;
	uint32_t root;

	[[nodiscard]] uint32_t create_node() {
		const auto id = pool.size();
		pool.emplace_back();
		return id;
	}

	[[nodiscard]] static SearchResult Node2Result(const Node &node) {
		return {.common = node.common, .endcnt = node.endcnt};
	}

	public:
	SparseTrie() : root(create_node()) {}

	template<std::ranges::input_range R, class RV = std::ranges::range_value_t<R>>
	requires std::ranges::input_range<RV> && convertible_to<std::ranges::range_value_t<RV>, T>
	SparseTrie(R &&r) : root(create_node()) {
		if constexpr(std::ranges::forward_range<R> && std::ranges::sized_range<RV>) {
			size_t maxsiz = 0;
			for(auto &&s : r) {
				maxsiz += std::ranges::size(s);
			}

			reserve(maxsiz);
		}

		auto v = views::all(std::forward<R>(r));

		for(auto &&s : v) {
			add(s);
		}
	}

	[[nodiscard]] size_t size() const {
		return pool[root].common;
	}

	void reserve(const size_t n) {
		pool.reserve(n);
	}

	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, T>
	void add(R &&r, const ll n = 1) {
		if constexpr(std::ranges::sized_range<R>) {
			reserve(pool.size() + std::ranges::size(r));
		}

		auto v = views::all(std::forward<R>(r));
		uint32_t node = root;

		pool[node].common += n;
		for(const T &c : v) {
			const auto child_it = pool[node].cs.find(c);
			if(child_it == pool[node].cs.end()) {
				node = pool[node].cs[c] = create_node();
			} else {
				node = child_it->second;
			}
			pool[node].common += n;
		}
		pool[node].endcnt += n;
	}

	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, T>
	void del(R &&r, const ll n = 1) {
		add(std::forward<R>(r), -n);
	}

	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, T>
	[[nodiscard]] SearchResult search(R &&r) const {
		auto v = views::all(std::forward<R>(r));
		uint32_t node = root;

		for(const T &c : v) {
			const auto child_it = pool[node].cs.find(c);
			if(child_it == pool[node].cs.end()) {
				return {0, 0};
			}
			node = child_it->second;
		}

		return Node2Result(pool[node]);
	}
};
