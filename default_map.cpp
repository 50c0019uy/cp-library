// https://github.com/50c0019uy/cp-library/blob/main/defualt_map.cpp

#include <concepts>
#include <map>
#include <unordered_map>

using namespace std;

template<auto defval, class... Args>
requires convertible_to<decltype(defval), typename map<Args...>::mapped_type>
class default_map : public map<Args...> {
	using Map = map<Args...>;
	template<class U>
	static constexpr bool comparable = requires (Map::key_compare c, Map::key_type t, U u) {
		c(t, u);
		c(u, t);
	};

	public:
#ifdef __cpp_lib_associative_heterogeneous_insertion
	template<convertible_to<typename Map::key_type> U>
	requires comparable<U>
	Map::mapped_type &operator[](const U &key) {
		Map::try_emplace(key, defval);
		return Map::operator[](key);
	}

	template<convertible_to<typename Map::key_type> U>
	requires comparable<U>
	Map::mapped_type &operator[](const U &&key) {
		Map::try_emplace(key, defval);
		return Map::operator[](key);
	}
#else
	Map::mapped_type &operator[](const Map::key_type &key) {
		Map::try_emplace(key, defval);
		return Map::operator[](key);
	}

	Map::mapped_type &operator[](const Map::key_type &&key) {
		Map::try_emplace(key, defval);
		return Map::operator[](key);
	}
#endif
};

template<auto defval, class... Args>
requires convertible_to<decltype(defval), typename unordered_map<Args...>::mapped_type>
class default_unordered_map : public unordered_map<Args...> {
	using Map = unordered_map<Args...>;
	template<class U>
	static constexpr bool hashable = requires (Map::hasher h, U u) { h(u); };

	public:
#ifdef __cpp_lib_associative_heterogeneous_insertion
	template<class U>
	requires hashable<U>
	Map::mapped_type &operator[](const U &key) {
		Map::try_emplace(key, defval);
		return Map::operator[](key);
	}

	template<class U>
	requires hashable<U>
	Map::mapped_type &operator[](const U &&key) {
		Map::try_emplace(key, defval);
		return Map::operator[](key);
	}
#else
	Map::mapped_type &operator[](const Map::key_type &key) {
		Map::try_emplace(key, defval);
		return Map::operator[](key);
	}

	Map::mapped_type &operator[](const Map::key_type &&key) {
		Map::try_emplace(key, defval);
		return Map::operator[](key);
	}
#endif
};
