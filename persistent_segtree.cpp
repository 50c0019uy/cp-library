// https://github.com/50c0019uy/cp-library/blob/main/persistent_segtree.cpp

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include "range/v3/view/repeat_n.hpp"

using namespace std;

template<class T, auto op, auto e>
class persistent_segtree {
	static_assert(
		requires (T a, T b) {
			{ op(a, b) } -> convertible_to<T>;
			{ e() } -> convertible_to<T>;
		}, "requires not satisfied");

	struct Node {
		const uint32_t val = 0;
		const uint32_t left = 0;
		const uint32_t right = 0;
	};

	private:
	static vector<Node> nodes;
	static vector<T> vals;

	uint32_t root;
	uint32_t root_depth;

	static T _prod(
		const uint32_t node, const uint32_t l, const uint32_t r, const uint32_t ql, const uint32_t qr) {
		if(qr <= l || r <= ql) return vals[0];
		if(ql <= l && r <= qr) return vals[nodes[node].val];

		const auto m = midpoint(l, r);
		T left_val = _prod(nodes[node].left, l, m, ql, qr);
		T right_val = _prod(nodes[node].right, m, r, ql, qr);
		return op(std::move(left_val), std::move(right_val));
	}

	static uint32_t _set(const uint32_t node, const uint32_t i, const uint32_t depth, T &&val) {
		if(depth == 0) {
			vals.push_back(std::move(val));
			nodes.emplace_back(vals.size() - 1, 0, 0);
			return nodes.size() - 1;
		}

		const bool is_next_left = (i >> (depth - 1) & 1) == 0;
		const auto left =
			is_next_left ? _set(nodes[node].left, i, depth - 1, std::move(val)) : nodes[node].left;
		const auto right =
			not is_next_left ? _set(nodes[node].right, i, depth - 1, std::move(val)) : nodes[node].right;

		vals.push_back(op(vals[nodes[left].val], vals[nodes[right].val]));
		const auto new_val = vals.size() - 1;
		nodes.emplace_back(new_val, left, right);

		return nodes.size() - 1;
	}

	persistent_segtree(
		[[maybe_unused]] Node /*unused*/, const uint32_t _root, const uint32_t _root_depth)
		: root(_root), root_depth(_root_depth) {}

	public:
	static void reserve(const size_t n) {
		nodes.reserve(nodes.size() + n);
		vals.reserve(vals.size() + n);
	}

	persistent_segtree() : root(0), root_depth(0) {}

	persistent_segtree(const size_t _n) : persistent_segtree(_n, vals[0]) {}

	persistent_segtree(const size_t _n, const T &v)
		: persistent_segtree(::ranges::views::repeat_n(v, _n)) {}

	persistent_segtree(const size_t _n, T &&v)
		: persistent_segtree(::ranges::views::repeat_n(std::move(v), _n)) {}

	template<std::ranges::input_range R>
	requires convertible_to<std::ranges::range_value_t<R>, T>
	persistent_segtree(R &&r) {
		if constexpr(std::ranges::sized_range<R>) {
			const auto n = std::ranges::size(r);
			nodes.reserve(nodes.size() + n * 2 + bit_width(n));
			vals.reserve(vals.size() + n * 2 + bit_width(n));
		}
		const auto former_size = vals.size();
		size_t n = 0;
		for(auto &&elem : r) {
			if constexpr(is_rvalue_reference_v<R &&>) {
				vals.push_back(std::move(elem));
			} else {
				vals.push_back(std::forward<decltype(elem)>(elem));
			}
			n++;
		}

		if(n == 0) {
			root = 0;
			root_depth = 0;
			return;
		}

		const auto build = [n, former_size](
												 auto &&self, const uint32_t i, const uint32_t d) -> uint32_t {
			if(i >= n) return 0;
			if(d == 0) {
				nodes.emplace_back(former_size + i, 0, 0);
				return nodes.size() - 1;
			}

			const auto left = self(self, i, d - 1);
			const auto right = self(self, i | (uint32_t(1) << (d - 1)), d - 1);
			vals.push_back(op(vals[nodes[left].val], vals[nodes[right].val]));
			const auto val = vals.size() - 1;
			nodes.emplace_back(val, left, right);
			return nodes.size() - 1;
		};
		root = build(build, 0, bit_width(n - 1));
		root_depth = bit_width(n - 1);
	}

	persistent_segtree(const persistent_segtree &) = default;

	persistent_segtree(persistent_segtree &&t) noexcept
		: persistent_segtree(static_cast<const persistent_segtree &>(t)) {}

	persistent_segtree &operator=(const persistent_segtree &) = default;

	persistent_segtree &operator=(persistent_segtree &&t) noexcept {
		return *this = static_cast<const persistent_segtree &>(t);
	}

	~persistent_segtree() = default;

	[[nodiscard]] const T &get(const uint32_t i) const {
		assert(i < (uint32_t(1) << root_depth));
		uint32_t node = root;

		for(uint32_t depth = root_depth; depth > 0; depth--) {
			if((i >> (depth - 1) & 1) == 0) {
				node = nodes[node].left;
			} else {
				node = nodes[node].right;
			}
		}

		return vals[nodes[node].val];
	}

	[[nodiscard]] const T &operator[](const uint32_t i) const {
		assert(i < (uint32_t(1) << root_depth));
		return get(i);
	}

	// [l, r)
	[[nodiscard]] T prod(const uint32_t l, const uint32_t r) const {
		assert(l <= r);
		assert(r <= (uint32_t(1) << root_depth));
		return _prod(root, 0, uint32_t(1) << root_depth, l, r);
	}

	[[nodiscard]] T all_prod() const {
		return vals[nodes[root].val];
	}

	[[nodiscard]] persistent_segtree set(const uint32_t i, const T &val) const {
		assert(i < (uint32_t(1) << root_depth));
		return set(i, T(val));
	}

	[[nodiscard]] persistent_segtree set(const uint32_t i, T &&val) const {
		assert(i < (uint32_t(1) << root_depth));
		return persistent_segtree(Node{}, _set(root, i, root_depth, std::move(val)), root_depth);
	}
};

template<class T, auto op, auto e>
vector<typename persistent_segtree<T, op, e>::Node> persistent_segtree<T, op, e>::nodes(1);

template<class T, auto op, auto e>
vector<T> persistent_segtree<T, op, e>::vals(1, e());

// template class persistent_segtree<int, [](int a, int b) { return a + b; }, [] { return 0; }>;
