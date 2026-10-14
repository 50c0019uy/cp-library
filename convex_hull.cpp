// https://github.com/50c0019uy/cp-library/blob/main/convex_hull.cpp

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

using namespace std;

using ll = long long;
using point = pair<ll, ll>;

// yosupo judge checker verified.
// https://judge.yosupo.jp/submission/327383

// clockwise
vector<point> convex_hull(vector<point> v) {
	const auto minus = [](const point &a, const point &b) {
		return point{a.first - b.first, a.second - b.second};
	};
	const auto cross = [](const point &a, const point &b) {
		return a.first * b.second - b.first * a.second;
	};
	if(v.empty()) return v;
	std::ranges::sort(v);
	v.erase(get<0>(std::ranges::unique(v)), v.end());
	vector<point> uh, dh;
	for(const auto &e : v) {
		const auto lst = [](const auto &v) { return v[v.size() - 1]; };
		const auto lst2 = [](const auto &v) { return v[v.size() - 2]; };
		while(uh.size() >= 2 && cross(minus(lst(uh), lst2(uh)), minus(e, lst(uh))) >= 0) {
			uh.pop_back();
		}
		while(dh.size() >= 2 && cross(minus(lst(dh), lst2(dh)), minus(e, lst(dh))) <= 0) {
			dh.pop_back();
		}
		uh.push_back(e);
		dh.push_back(e);
	}

	if(dh.size() > 2) uh.insert(uh.end(), next(dh.rbegin()), prev(dh.rend()));
	return uh;
}
