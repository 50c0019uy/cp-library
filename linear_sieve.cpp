#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

using namespace std;

using ll = long long;

// [1, n]の最小素因数(v[0] = 0, v[1] = 1), n以下の素数.
// O(n)
class linear_sieve {
	public:
	linear_sieve(const ll n) {
		minfactor.resize(n + 1, 0);
		minfactor[0] = 0;
		minfactor[1] = 1;
		primes.reserve(ll(n / log(n) * 1.15));
		for(ll i = 2; i <= n; i++) {
			if(! minfactor[i]) minfactor[i] = i, primes.push_back(i);

			for(size_t j = 0; j != primes.size() && primes[j] <= minfactor[i]; j++) {
				if(i * primes[j] > n) break;
				minfactor[i * primes[j]] = primes[j];
			}
		}
	}

	vector<ll> minfactor, primes;
};

// sqrt(R)以下の素数、[L, R]の素数判定.
// O(sqrt(R) + (R - L)loglog(R - L))
class segment_sieve : public linear_sieve {
	public:
	segment_sieve(const ll l, const ll r) : linear_sieve((ll)sqrt(r) + 10) {
		isprime = vector<bool>(r - l + 1, true);
		for(const auto &p : primes) {
			ll cur = max(p * p, (l + p - 1) / p * p) - l;
			for(; cur < r - l + 1; cur += p) {
				isprime[cur] = false;
			}
		}
	}

	vector<bool> isprime;
};
