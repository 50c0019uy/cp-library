// https://github.com/50c0019uy/cp-library/blob/main/cartesian_tree.cpp

#include <cassert>
#include <functional>
#include <ranges>
#include <vector>

using namespace std;
using ll = long long;

// comp(child, parent) == true
template<auto comp = less<>{}>
struct cartesian_tree {
	static constexpr ll null = -1;

	ll root;
	vector<ll> lch, rch;

	cartesian_tree(const vector<ll> &v) : root(0) {
		const ll n = v.size();
		lch.resize(n, null);
		rch.resize(n, null);
		vector<ll> st;
		st.push_back(0);
		for(const auto i : views::iota(1, n)) {
			while(not st.empty()) {
				const ll r = st.back();
				if(comp(v[i], v[r])) {
					rch[r] = i;
					break;
				}
				st.pop_back();
				lch[i] = r;
			}
			if(st.empty()) {
				root = i;
			}
			st.push_back(i);
		}
	}
};
