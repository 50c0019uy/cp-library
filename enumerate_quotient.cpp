#include <cassert>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <tuple>

using namespace std;

using ll = long long;

class enumerate_quotient_view : public std::ranges::view_interface<enumerate_quotient_view> {
	private:
	ll n;

	public:
	class iterator {
		private:
		ll n, l;

		public:
		using value_type = tuple<ll, ll, ll>;
		using difference_type = ptrdiff_t;
		using iterator_concept = forward_iterator_tag;

		iterator() = default;

		iterator(ll _n, ll _l) : n(_n), l(_l) {}

		value_type operator*() const {
			const ll q = n / l;
			const ll r = n / q + 1;
			return {q, l, r};
		}

		iterator &operator++() {
			const ll q = n / l;
			const ll r = n / q + 1;
			l = r;
			return *this;
		}

		iterator operator++(int) {
			iterator ret(*this);
			++(*this);
			return ret;
		}

		bool operator==(const iterator &) const = default;
	};

	static_assert(forward_iterator<iterator>);

	enumerate_quotient_view(ll _n) : n(_n) {
		assert(n >= 1);
	}

	[[nodiscard]] iterator begin() const {
		return iterator{n, 1};
	}

	[[nodiscard]] iterator end() const {
		return iterator{n, n + 1};
	}
};

static_assert(std::ranges::view<enumerate_quotient_view>);
static_assert(std::ranges::forward_range<enumerate_quotient_view>);
