// https://github.com/50c0019uy/cp-library/blob/main/treap.cpp

#include <cassert>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "range/v3/view/repeat_n.hpp"

using namespace std;
using ull = unsigned long long;

ull randint(ull, ull);

template<class T,
	auto op,
	auto e = [] { return T{}; },
	class F = monostate,
	auto mapping = nullptr,
	auto composition = nullptr,
	auto id = [] { return F{}; }>
requires requires (T a, T b, F f, F g) {
	{ op(a, b) } -> convertible_to<T>;
	{ e() } -> convertible_to<T>;
	requires mapping == nullptr || requires () {
		{ mapping(f, a) } -> convertible_to<T>;
	};
	requires composition == nullptr || requires () {
		{ composition(f, g) } -> convertible_to<F>;
	};
	{ id() } -> convertible_to<F>;
}
class treap {
	private:
	struct Node {
		T val = e();
		T sum = e();
		T rsum = e();
		F lazy = id();
		bool rev = false;
		uint32_t pri = 0;
		uint32_t size = 0;
		uint32_t lchild = null;
		uint32_t rchild = null;
		static constexpr uint32_t null = 0;
	};

	static constexpr bool can_lazy = mapping != nullptr && composition != nullptr;

	void heapify(const uint32_t i) {
		if(isnull(i)) return;
		uint32_t mai = i;
		if(const uint32_t lch = nodes[i].lchild; not isnull(lch) && nodes[mai].pri < nodes[lch].pri)
			mai = lch;
		if(const uint32_t rch = nodes[i].rchild; not isnull(rch) && nodes[mai].pri < nodes[rch].pri)
			mai = rch;
		if(mai == i) return;
		swap(nodes[mai].pri, nodes[i].pri);
		heapify(mai);
	}

	uint32_t build(input_iterator auto &it, const uint32_t l, const uint32_t r) {
		if(r - l == 0) return Node::null;
		const uint32_t mid = midpoint(l, r);
		const uint32_t lch = build(it, l, mid);
		const uint32_t node = create_node(std::forward<T>(*it));
		it++;
		const uint32_t rch = build(it, mid + 1, r);
		nodes[node].lchild = lch;
		nodes[node].rchild = rch;
		update(node);
		heapify(node);
		return node;
	}

	public:
	treap(const uint32_t n = 0) : treap(::ranges::views::repeat_n(e(), n)) {}

	template<std::ranges::sized_range R>
	requires std::ranges::input_range<R> && convertible_to<std::ranges::range_value_t<R>, T>
	         && (not same_as<remove_cvref_t<std::ranges::range_value_t<R>>, T>)
	treap(R &&r) : treap(r | views::transform([](auto &&v) { return static_cast<T>(v); })) {}

	template<std::ranges::sized_range R>
	requires std::ranges::input_range<R> && same_as<remove_cvref_t<std::ranges::range_value_t<R>>, T>
	treap(R &&r) {
		auto it = std::ranges::begin(r);
		if constexpr(is_rvalue_reference_v<R &&>) {
			it = make_move_iterator(it);
		}
		root = build(it, 0, std::ranges::size(r));
	}

	private:
	uint32_t merge(const uint32_t i, const uint32_t j) {
		if(isnull(i)) {
			if(not isnull(j)) push(j);
			return j;
		}
		if(isnull(j)) {
			if(not isnull(i)) push(i);
			return i;
		}

		if(nodes[i].pri > nodes[j].pri) {
			push(i);
			nodes[i].rchild = merge(nodes[i].rchild, j);
			return update(i);
		} else {
			push(j);
			nodes[j].lchild = merge(i, nodes[j].lchild);
			return update(j);
		}
	}

	// [0, k), [k, size)
	pair<uint32_t, uint32_t> split(const uint32_t i, const uint32_t k) {
		if(isnull(i)) return {Node::null, Node::null};

		push(i);
		Node &node = nodes[i];
		const uint32_t lsize = nodes[node.lchild].size;
		if(k <= lsize) {
			const auto [l, r] = split(node.lchild, k);
			node.lchild = r;
			return {l, update(i)};
		} else {
			const auto [l, r] = split(node.rchild, k - lsize - 1);
			node.rchild = l;
			return {update(i), r};
		}
	}

	public:
	void insert(const uint32_t k, const T &v) {
		assert(0 <= k && k <= size());
		const auto [l, r] = split(root, k);
		root = merge(merge(l, create_node(v)), r);
	}

	void erase(const uint32_t k) {
		assert(0 <= k && k < size());
		erase(k, k + 1);
	}

	void erase(const uint32_t l, const uint32_t r) {
		assert(0 <= l && l <= r && r <= size());
		if(l == r) return;
		const auto [lt, tmp] = split(root, l);
		const auto [_, rt] = split(tmp, r - l);
		root = merge(lt, rt);
	}

	private:
	[[nodiscard]] const T &get(const uint32_t i, const uint32_t k) {
		push(i);
		Node &node = nodes[i];
		const uint32_t lsize = nodes[node.lchild].size;
		if(k == lsize) {
			return node.val;
		}
		if(k < lsize) {
			return get(node.lchild, k);
		} else {
			return get(node.rchild, k - lsize - 1);
		}
	}

	public:
	const T &operator[](const uint32_t k) {
		assert(0 <= k && k < size());
		return get(root, k);
	}

	private:
	void _set(const uint32_t i, const uint32_t k, const T &v) {
		push(i);
		Node &node = nodes[i];
		const uint32_t lsize = nodes[node.lchild].size;
		if(k == lsize) {
			node.val = v;
			update(i);
			return;
		}
		if(k < lsize) {
			_set(node.lchild, k, v);
		} else {
			_set(node.rchild, k - lsize - 1, v);
		}
		update(i);
	}

	public:
	void set(const uint32_t k, const T &v) {
		assert(0 <= k && k < size());
		_set(root, k, v);
	}

	void reverse(const uint32_t l, const uint32_t r) {
		assert(0 <= l && l <= r && r <= size());
		const auto [tmp, rt] = split(root, r);
		const auto [lt, mid] = split(tmp, l);
		nodes[mid].rev = ! nodes[mid].rev;
		root = merge(merge(lt, mid), rt);
	}

	void all_reverse() {
		if(size() > 0) {
			nodes[root].rev ^= true;
		}
	}

	private:
	template<auto Sum>
	[[nodiscard]] T _prod_abs(const uint32_t i, const uint32_t k) {
		if(k == 0) return e();
		push(i);

		const uint32_t lch = nodes[i].lchild;
		const uint32_t rch = nodes[i].rchild;
		const uint32_t lsize = nodes[lch].size;
		if(k <= lsize) return _prod_abs<Sum>(lch, k);
		if(not isnull(lch)) push(lch);
		T res = e();
		res = op(res, nodes[lch].*Sum);
		res = op(res, nodes[i].val);
		res = op(res, _prod_abs<Sum>(rch, k - lsize - 1));
		return res;
	}

	public:
	[[nodiscard]] T prod(const uint32_t l, const uint32_t r) {
		assert(0 <= l && l <= r && r <= size());
		const auto [lt, rt] = split(root, l);
		const T res = _prod_abs<&Node::sum>(rt, r - l);
		root = merge(lt, rt);
		return res;
	}

	[[nodiscard]] T rprod(const uint32_t l, const uint32_t r) {
		assert(0 <= l && l <= r && r <= size());
		const auto [lt, rt] = split(root, l);
		const T res = _prod_abs<&Node::rsum>(rt, r - l);
		root = merge(lt, rt);
		return res;
	}

	[[nodiscard]] const T &all_prod() {
		push(root);
		return nodes[root].sum;
	}

	[[nodiscard]] const T &all_rprod() {
		push(root);
		return nodes[root].rsum;
	}

	void apply(const uint32_t k, const F &f)
	requires (can_lazy)
	{
		assert(0 <= k && k < size());
		apply(k, k + 1, f);
	}

	private:
	void _apply(const uint32_t i, const uint32_t k, const F &f)
	requires (can_lazy)
	{
		if(k == 0) return;

		push(i);
		const uint32_t lch = nodes[i].lchild;
		const uint32_t rch = nodes[i].rchild;
		const uint32_t lsize = nodes[lch].size;
		if(k <= lsize) {
			_apply(lch, k, f);
		} else {
			if(not isnull(lch)) nodes[lch].lazy = composition(nodes[lch].lazy, f);
			nodes[i].val = mapping(f, nodes[i].val);
			_apply(rch, k - lsize - 1, f);
		}
		update(i);
	}

	public:
	void apply(const uint32_t l, const uint32_t r, const F &f)
	requires (can_lazy)
	{
		assert(0 <= l && l <= r && r <= size());
		const auto [lt, rt] = split(root, l);
		_apply(rt, r - l, f);
		root = merge(lt, rt);
	}

	void all_apply(const F &f)
	requires (can_lazy)
	{
		if(size() > 0) {
			nodes[root].lazy = composition(nodes[root].lazy, f);
		}
	}

	[[nodiscard]] uint32_t size() const noexcept {
		return nodes[root].size;
	}

	private:
	void _output_tree(const uint32_t i) noexcept {
		if(isnull(i)) return;
		push(i);
		if(const uint32_t child = nodes[i].lchild; not isnull(child))
			cout << i << " " << child << "\n", _output_tree(child);
		if(const uint32_t child = nodes[i].rchild; not isnull(child))
			cout << i << " " << child << "\n", _output_tree(child);
	}

	public:
	void _output_tree() noexcept {
		_output_tree(root);
	}

	private:
	uint32_t _max_right(const uint32_t i, const T &csum, const auto &f) {
		if(isnull(i)) return 0;

		push(i);
		const auto &node = nodes[i];
		if(! isnull(node.lchild)) push(node.lchild);
		const T &ltsum = nodes[node.lchild].sum;
		if(const T lsum = op(csum, ltsum); ! f(lsum)) {
			return _max_right(node.lchild, csum, f);
		} else if(const T lmsum = op(lsum, node.val); ! f(lmsum)) {
			return nodes[node.lchild].size;
		} else {
			return nodes[node.lchild].size + 1 + _max_right(node.rchild, lmsum, f);
		}
	}

	public:
	uint32_t max_right(const uint32_t l, const auto &f) {
		assert(0 <= l && l <= size());
		assert(f(e()));

		const auto [lt, rt] = split(root, l);
		const uint32_t res = l + _max_right(rt, e(), f);
		root = merge(lt, rt);
		return res;
	}

	private:
	uint32_t _min_left(const uint32_t i, const T &csum, const auto &f) {
		if(isnull(i)) return 0;

		push(i);
		const auto &node = nodes[i];
		if(! isnull(node.rchild)) push(node.rchild);
		const T &rtsum = nodes[node.rchild].sum;
		if(const T rsum = op(rtsum, csum); ! f(rsum)) {
			return _min_left(node.rchild, csum, f);
		} else if(const T rmsum = op(node.val, rsum); ! f(rmsum)) {
			return nodes[node.rchild].size;
		} else {
			return nodes[node.rchild].size + 1 + _min_left(node.lchild, rmsum, f);
		}
	}

	public:
	uint32_t min_left(const uint32_t r, const auto &f) {
		static_assert(e != nullptr, "e() must not be nullptr");
		assert(0 <= r && r <= size());
		assert(f(e()));

		const auto [lt, rt] = split(root, r);
		const uint32_t res = r - _min_left(lt, e(), f);
		root = merge(lt, rt);
		return res;
	}

	private:
	[[nodiscard]] bool isnull(const uint32_t i) const noexcept {
		return i == Node::null;
	}

	[[nodiscard]] uint32_t create_node(const T &v) {
		nodes.push_back(Node{.val = v,
			.sum = v,
			.rsum = v,
			.lazy = id(),
			.rev = false,
			.pri = (uint32_t)randint(0, numeric_limits<uint32_t>::max()),
			.size = 1});
		return nodes.size() - 1;
	}

	uint32_t update(const uint32_t i) noexcept {
		Node &node = nodes[i];
		const Node &lch = nodes[node.lchild];
		const Node &rch = nodes[node.rchild];
		if(not isnull(node.lchild)) push(node.lchild);
		if(not isnull(node.rchild)) push(node.rchild);
		node.size = 1 + lch.size + rch.size;
		node.sum = op(op(lch.sum, node.val), rch.sum);
		node.rsum = op(op(rch.rsum, node.val), lch.rsum);
		return i;
	}

	void push(const uint32_t i) noexcept {
		Node &node = nodes[i];
		if(node.rev) {
			node.rev = false;
			swap(node.lchild, node.rchild);
			swap(node.sum, node.rsum);
			if(not isnull(node.lchild)) nodes[node.lchild].rev ^= true;
			if(not isnull(node.rchild)) nodes[node.rchild].rev ^= true;
		}
		if constexpr(can_lazy) {
			const uint32_t lch = node.lchild;
			const uint32_t rch = node.rchild;
			node.val = mapping(node.lazy, node.val);
			node.sum = mapping(node.lazy, node.sum);
			node.rsum = mapping(node.lazy, node.rsum);
			if(not isnull(lch)) nodes[lch].lazy = composition(nodes[lch].lazy, node.lazy);
			if(not isnull(rch)) nodes[rch].lazy = composition(nodes[rch].lazy, node.lazy);
			node.lazy = id();
		}
	}

	uint32_t root = Node::null;
	vector<Node> nodes{1};
};

#include <atcoder/modint.hpp>

using mint = atcoder::modint998244353;
using T = pair<mint, mint>;
using F = mint;

const auto op = [](const T &a, const T &b) { return T{a.first + b.first, a.second + b.second}; };

const auto e = [] { return T{0, 0}; };

const auto mapping = [](const T &a, const F &f) { return T{a.first + f, a.second + f}; };

const auto composition = [](const F &f, const F &g) { return f + g; };

const auto id = [] { return F{0}; };

template class treap<T, op, e, F, mapping, composition, id>;
