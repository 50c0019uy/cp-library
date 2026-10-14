// https://github.com/50c0019uy/cp-library/blob/main/argument_sort.cpp

#include <algorithm>
#include <utility>
#include <vector>

using namespace std;
using ll = long long;
using pall = pair<ll, ll>;

bool argument_comp(pall p, pall q) {
	const bool is_p_lower = p.second < 0 || (p.second == 0 && p.first < 0);
	const bool is_q_lower = q.second < 0 || (q.second == 0 && q.first < 0);
	if(is_p_lower != is_q_lower) {
		return static_cast<int>(is_p_lower) < static_cast<int>(is_q_lower);
	}

	const ll cross = p.first * q.second - q.first * p.second;
	return 0 < cross;
}

template<class... Args>
void argument_sort(vector<pall> &v, Args &&...args) {
	std::ranges::sort(v, argument_comp, std::forward<Args>(args)...);
}
