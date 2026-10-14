// https://github.com/50c0019uy/cp-library/blob/main/persistent_stack.cpp

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

using namespace std;

template<class T>
class persistent_stack {
	public:
	static_assert(is_default_constructible_v<T>, "T must be default constructible");

	struct Node {
		const T val{};
		const size_t next = 0;
	};

	private:
	static vector<Node> nodes;

	size_t node;
	size_t count;

	persistent_stack(const size_t _node, const size_t _count) : node(_node), count(_count) {}

	public:
	static void reserve(const size_t n) {
		nodes.reserve(n);
	}

	persistent_stack() : node(0), count(0) {}

	[[nodiscard]] size_t size() const {
		return count;
	}

	[[nodiscard]] bool empty() const {
		return size() == 0;
	}

	[[nodiscard]] const T &top() const {
		assert(not empty());
		return nodes[node].val;
	}

	[[nodiscard]] persistent_stack push(const T &val) const {
		return push(T(val));
	}

	[[nodiscard]] persistent_stack push(T &&val) const {
		nodes.emplace_back(std::move(val), node);
		return persistent_stack(nodes.size() - 1, count + 1);
	}

	[[nodiscard]] persistent_stack pop() const {
		assert(not empty());
		return persistent_stack(nodes[node].next, count - 1);
	}
};

template<class T>
vector<typename persistent_stack<T>::Node> persistent_stack<T>::nodes(1);

// template class persistent_stack<int>;
