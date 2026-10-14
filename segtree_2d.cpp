// https://github.com/50c0019uy/cp-library/blob/main/segtree_2d.cpp

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>

using namespace std;

using pall = pair<long long, long long>;

#define check_bound(var, lower, upper) \
	assert((lower) <= (var)); \
	assert((var) <= (upper));

namespace myclass {
template<class S, S (*op)(S, S), S (*e)()>
struct segt {
	segt(const size_t _n = 0) : n(_n), size(bit_ceil(_n)) {
		if(n != 0) {
			v = decltype(v)(n + size, e());
		}
	}

	segt(const intmax_t _n) : n(_n), size(bit_ceil<uint64_t>(abs(_n))) {
		if(n != 0) {
			v = decltype(v)(n + size, e());
		}
	}

	segt &operator=(const segt &t) = default;

	segt &operator=(segt &&t) noexcept = default;

	void set(int p, const S val) {
		check_bound(p, 0, n - 1);
		p += size;
		v[p] = val;
		p /= 2;
		while(p != 0) {
			update(p);
			p /= 2;
		}
	}

	S get(const int p) const {
		check_bound(p, 0, n - 1);
		return v[p + size];
	}

	S prod(int l, int r) const {
		check_bound(l, 0, r);
		check_bound(r, l, n);
		l += size;
		r += size;
		S lval = e(), rval = e();
		while(l < r) {
			if(l & 1) lval = op(lval, v[l]), l++;
			if(r & 1) r--, rval = op(v[r], rval);
			l /= 2;
			r /= 2;
		}
		return op(lval, rval);
	}

	S all_prod() const {
		return v[1];
	}

	vector<S> v;
	size_t n, size;

	private:
	void update(const int i) {
		if(2 * i + 1 < v.size()) {
			v[i] = op(v[2 * i], v[2 * i + 1]);
		} else {
			v[i] = v[2 * i];
		}
	}
};
}  // namespace myclass

template<class S, S (*op)(S, S), S (*e)()>
class segtree_2d {
	public:
	segtree_2d(const int _h, const int _w) : h(_h), w(_w), size(bit_ceil<uint64_t>(_h)) {
		v = decltype(v)(h + size, w);
	}

	void set(int i, const int j, const S val) {
		check_bound(i, 0, h - 1);
		check_bound(j, 0, w - 1);
		i += size;
		v[i].set(j, val);
		i /= 2;
		while(i != 0) {
			update(i, j);
			i /= 2;
		}
	}

	S get(int i, const int j) const {
		check_bound(i, 0, h - 1);
		check_bound(j, 0, w - 1);
		i += size;
		return v[i].get(j);
	}

	S prod(int i1, const int j1, int i2, const int j2) const {
		check_bound(i1, 0, i2);
		check_bound(i2, i1, h);
		check_bound(j1, 0, j2);
		check_bound(j2, j1, w);
		i1 += size;
		i2 += size;
		S ret = e();
		while(i1 < i2) {
			if(i1 & 1) ret = op(ret, v[i1].prod(j1, j2)), i1++;
			if(i2 & 1) i2--, ret = op(ret, v[i2].prod(j1, j2));
			i1 /= 2;
			i2 /= 2;
		}
		return ret;
	}

	S all_prod() const {
		return v[1].all_prod();
	}

	private:
	vector<myclass::segt<S, op, e>> v;
	const int h, w, size;

	void update(const int i, const int j) {
		if(2 * i + 1 < v.size()) {
			v[i].set(j, op(v[2 * i].get(j), v[2 * i + 1].get(j)));
		} else {
			v[i].set(j, v[2 * i].get(j));
		}
	}
};

template<class S, S (*op)(S, S), S (*e)()>
class segtree_2d_sparse {
	using segt = myclass::segt<S, op, e>;

	public:
	segtree_2d_sparse(const int _h, const int _w, const vector<pall> &ps)
		: h(_h), w(_w), size(bit_ceil<uint64_t>(_h)) {
		ws = decltype(ws)(h + size);
		v = decltype(v)(h + size);
		for(const auto &p : ps) {
			const int &i = p.first;
			const int &j = p.second;
			check_bound(i, 0, h - 1);
			check_bound(j, 0, w - 1);
			ws[i + size].push_back(j);
		}
		for(int i = 0; i < h; i++) {
			v[i + size] = segt(ws[i + size].size());
			std::ranges::sort(ws[i + size]);
		}
		for(int i = (h + size - 1) / 2; i > 0; i--) {
			if(2 * i + 1 >= h + size) {
				v[i] = v[2 * i];
				ws[i] = ws[2 * i];
				continue;
			}
			const int xm = ws[2 * i].size();
			const int ym = ws[2 * i + 1].size();
			int x = 0, y = 0;
			while(x < xm || y < ym) {
				const int xv = x < xm ? ws[2 * i][x] : w + 1;
				const int yv = y < ym ? ws[2 * i + 1][y] : w + 1;
				ws[i].push_back(min(xv, yv));
				if(xv <= yv) x++;
				if(xv >= yv) y++;
			}
			v[i] = segt(ws[i].size());
		}
	}

	void set(int i, const int j, const S val) {
		check_bound(i, 0, h - 1);
		check_bound(j, 0, w - 1);
		i += size;
		const int idx = search(i, j);
		assert(idx != -1);
		v[i].set(idx, val);
		i /= 2;
		while(i != 0) {
			update(i, j);
			i /= 2;
		}
	}

	S get(int i, const int j) const {
		check_bound(i, 0, h - 1);
		check_bound(j, 0, w - 1);
		i += size;
		const int idx = search(i, j);
		if(idx == -1) return e();
		return v[i].get(idx);
	}

	S prod(int i1, const int j1, int i2, const int j2) const {
		check_bound(i1, 0, i2);
		check_bound(i2, i1, h);
		check_bound(j1, 0, j2);
		check_bound(j2, j1, w);
		i1 += size;
		i2 += size;
		S ret = e();
		while(i1 < i2) {
			if(i1 & 1) {
				const auto lit = std::ranges::lower_bound(ws[i1], j1);
				const auto rit = std::ranges::lower_bound(ws[i1], j2);
				const int lidx = lit - ws[i1].begin();
				const int ridx = rit - ws[i1].begin();
				ret = op(ret, v[i1].prod(lidx, ridx));
				i1++;
			}
			if(i2 & 1) {
				i2--;
				const auto lit = std::ranges::lower_bound(ws[i2], j1);
				const auto rit = std::ranges::lower_bound(ws[i2], j2);
				const int lidx = lit - ws[i2].begin();
				const int ridx = rit - ws[i2].begin();
				ret = op(ret, v[i2].prod(lidx, ridx));
			}
			i1 /= 2;
			i2 /= 2;
		}
		return ret;
	}

	S all_prod() const {
		return v[1].all_prod();
	}

	private:
	vector<segt> v;
	vector<vector<int>> ws;
	const int h, w, size;

	[[nodiscard]] int search(const int i, const int j) const noexcept {
		const auto it = std::ranges::lower_bound(ws[i], j);
		if(it == ws[i].end() || *it != j) return -1;
		return it - ws[i].begin();
	}

	void update(const int i, const int j) {
		S res = e();
		if(const int idx = search(2 * i, j); idx != -1) {
			res = v[2 * i].get(idx);
		}
		if(2 * i + 1 < v.size()) {
			if(const int idx = search(2 * i + 1, j); idx != -1) {
				res = op(res, v[2 * i + 1].get(idx));
			}
		}
		v[i].set(search(i, j), res);
	}
};

#undef check_bound

// template class segtree_2d<int, [](int a, int b) {return a + b;}, [] {return 0;}>;
// template class segtree_2d_sparse<int, [](int a, int b) {return a + b;}, [] {return 0;}>;
