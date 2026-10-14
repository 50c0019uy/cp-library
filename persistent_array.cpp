// https://github.com/50c0019uy/cp-library/blob/main/persistent_array.cpp

#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include "range/v3/view/repeat_n.hpp"

using namespace std;

template<uint32_t k, class T>
class persistent_array {
	public:
	static_assert(k >= 2, "k must be 2 or greater");

	struct Node {
		const uint32_t val = val_null;
		const array<uint32_t, k> children = [] {
			array<uint32_t, k> a{};
			a.fill(node_null);
			return a;
		}();
	};

	private:
	static constexpr uint32_t val_null = numeric_limits<uint32_t>::max();
	static constexpr uint32_t node_null = 0;
	static vector<Node> nodes;
	static vector<T> vals;

	uint32_t root;

	persistent_array([[maybe_unused]] Node /*unused*/, const uint32_t _root) : root(_root) {}

	static uint32_t build(
		const uint32_t base, const uint32_t i, const uint32_t pw, const uint32_t n) {
		if(pw >= n * k) return node_null;
		if(i >= n) return node_null;
		if(i * k < pw && i + pw >= n) return node_null;

		array<uint32_t, k> children;
		for(uint32_t j = 0; j < k; j++) {
			children[j] = build(base, i + j * pw, pw * k, n);
		}
		nodes.emplace_back(base + i, children);
		return nodes.size() - 1;
	}

	static uint32_t _set(const uint32_t node, uint32_t i, T &&val) {
		if(i == 0) {
			vals.push_back(std::move(val));
			nodes.emplace_back(vals.size() - 1, nodes[node].children);
			return nodes.size() - 1;
		}

		auto new_children = nodes[node].children;
		new_children[i % k] = _set(new_children[i % k], i / k, std::move(val));
		nodes.emplace_back(nodes[node].val, new_children);
		return nodes.size() - 1;
	}

	public:
	static void reserve(const uint32_t n) {
		nodes.reserve(n * ceil((log(n + 1) + 1) / log(k)));
		vals.reserve(n);
	}

	persistent_array() : root(node_null) {}

	persistent_array(const uint32_t _n)
	requires default_initializable<T>
		: persistent_array(_n, T{}) {}

	persistent_array(const uint32_t _n, const T &v)
		: persistent_array(::ranges::views::repeat_n(v, _n)) {}

	persistent_array(const uint32_t _n, T &&v)
		: persistent_array(::ranges::views::repeat_n(std::move(v), _n)) {}

	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, T>
	persistent_array(R &&r) {
		if constexpr(std::ranges::sized_range<R>) {
			vals.reserve(vals.size() + std::ranges::size(r));
		}
		const uint32_t cur_size = nodes.size();

		uint32_t n = 0;
		for(auto &&e : r) {
			if constexpr(is_rvalue_reference_v<R &&>) {
				vals.push_back(std::move(e));
			} else {
				vals.push_back(std::forward<decltype(e)>(e));
			}
			n++;
		}

		root = build(cur_size, 0, 1, n);
	}

	persistent_array(const persistent_array &) = default;

	persistent_array(persistent_array &&a) noexcept
		: persistent_array(static_cast<const persistent_array &>(a)) {}

	persistent_array &operator=(const persistent_array &) = default;

	persistent_array &operator=(persistent_array &&a) noexcept {
		return *this = static_cast<const persistent_array &>(a);
	}

	~persistent_array() = default;

	[[nodiscard]] const T &get(uint32_t i) const {
		auto node = root;

		while(i) {
			node = nodes[node].children[i % k];
			i /= k;
		}

		if(nodes[node].val == val_null) {
			if constexpr(default_initializable<T>) {
				static T defval{};
				return defval;
			}
			assert(default_initializable<T>);
		}

		return vals[nodes[node].val];
	}

	[[nodiscard]] const T &operator[](const uint32_t i) const {
		return get(i);
	}

	[[nodiscard]] persistent_array set(const uint32_t i, const T &val) const {
		return persistent_array(Node{}, _set(root, i, val));
	}

	[[nodiscard]] persistent_array set(const uint32_t i, T &&val) const {
		return persistent_array(Node{}, _set(root, i, std::move(val)));
	}
};

template<uint32_t k, class T>
vector<typename persistent_array<k, T>::Node> persistent_array<k, T>::nodes{1};
template<uint32_t k, class T>
vector<T> persistent_array<k, T>::vals;

// template class persistent_array<2, int>;
// template class persistent_array<20, int>;
