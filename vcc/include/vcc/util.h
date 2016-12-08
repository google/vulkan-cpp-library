/*
* Copyright 2016 Google Inc. All Rights Reserved.

* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.apache.org/licenses/LICENSE-2.0

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef UTIL_H_
#define UTIL_H_

#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <type/supplier.h>
#include <vector>

#if defined(_WIN32) && !defined(VK_USE_PLATFORM_WIN32_KHR)
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif // _WIN32
#if (defined(__ANDROID__) || defined(ANDROID)) && !defined(VK_USE_PLATFORM_ANDROID_KHR)
#define VK_USE_PLATFORM_ANDROID_KHR 1
#endif // __ANDROID__

#define VK_PROTOTYPES 1
#include <vulkan/vulkan.h>
#include <vcc/export.h>

namespace vcc {

// TODO(gardell): Rename to exception, perhaps add more types of exceptions.
class vcc_exception : public std::logic_error {
public:
	vcc_exception(const char *what) : std::logic_error(what) {}
	vcc_exception(const std::string &what) : std::logic_error(what) {}

	VCC_LIBRARY static void maybe_throw(VkResult result, const char *file, int line, const char *function, const char *expr);
};

namespace util {

template<typename U, typename V>
struct hash_pair {

	std::size_t operator()(const std::pair<U, V> &pair) const {
		return hash1(pair.first) * 31 + hash2(pair.second);
	}

	std::hash<U> hash1;
	std::hash<V> hash2;
};

template<std::size_t N>
struct tuple_foreach_type {
	template<typename FunctorT, typename... Args>
	static void call(FunctorT &functor, std::tuple<Args...> &args) {
		tuple_foreach_type<N - 1>::call(functor, args);
		functor(std::get<N - 1>(args));
	}
};

template<>
struct tuple_foreach_type<0> {
	template<typename FunctorT, typename... Args>
	static void call(FunctorT &, std::tuple<Args...> &) {}
};

template<typename FunctorT, typename... Args>
void tuple_foreach(FunctorT functor, std::tuple<Args...> &commands) {
	tuple_foreach_type<sizeof...(Args)>::call(functor, commands);
}

VCC_LIBRARY std::vector<const char *> to_pointers(const std::vector<std::string> &vector);
VCC_LIBRARY std::vector<const char *> to_pointers(const std::set<std::string> &set);

// Returns a vector with the given movable-only arguments added in the
// given order. Note that initializer-lists require copyable types.
template<typename T, typename... ValuesT>
std::vector<T> vector_from_variadic_movables(ValuesT &&...values) {
	std::vector<T> vector;
	vector.reserve(sizeof...(ValuesT));
	// dummy integer array guarantees order of evaluation on early GCC versions (bug).
	const int dummy[] = { (vector.push_back(std::forward<ValuesT>(values)), 0)... };
	return std::move(vector);
}

// Returns a set with the given movable-only arguments added in the
// given order. Note that initializer-lists require copyable types.
template<typename T, typename... ValuesT>
std::set<T> set_from_variadic_movables(ValuesT &&...values) {
	// Can't use initializer-list in these cases.
	std::set<T> set;
	// dummy integer array guarantees order of evaluation on early GCC versions (bug).
	const int dummy[] = { (set.insert(std::forward<ValuesT>(values)), 0)... };
	return std::move(set);
}

VCC_LIBRARY void diagnostic_print(const char *filename, const char *function, int line, const char *fmt, ...);

// TODO(gardell): Temporary fix for order-independent locking.
// Boost provides an order-independent lock for iterators
// but C++11 only for variadic arguments.
// We need order-independent even for dynamic arguments.
// std::vector for now as we require RandomAccessIterator.
template<typename LockableT>
void lock(std::vector<LockableT> &locks) {
	switch (locks.size()) {
	case 0:
		break;
	case 1:
		locks.front().lock();
		break;
	case 2:
		std::lock(locks[0], locks[1]);
		break;
	case 3:
		std::lock(locks[0], locks[1], locks[2]);
		break;
	case 4:
		std::lock(locks[0], locks[1], locks[2], locks[3]);
		break;
	case 5:
		std::lock(locks[0], locks[1], locks[2], locks[3], locks[4]);
		break;
	case 6:
		std::lock(locks[0], locks[1], locks[2], locks[3], locks[4], locks[5]);
		break;
	case 7:
		std::lock(locks[0], locks[1], locks[2], locks[3], locks[4], locks[5],
			locks[6]);
		break;
	case 8:
		std::lock(locks[0], locks[1], locks[2], locks[3], locks[4], locks[5],
			locks[6], locks[7]);
		break;
	default:
		throw std::logic_error("Can't lock more than 8 Lockable while "
			"guaranteeing order-independent locking! Feel free to add more cases.");
	}
}

namespace internal {

struct pass {
	template<typename ...T> pass(T...) {}
};

}  // namespace internal

}  // namespace util
}  // namespace vcc

#ifdef __GNUC__
#define VCC_PRINT(fmt, ...) vcc::util::diagnostic_print(__FILE__, \
	__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__);
#else
#define VCC_PRINT(fmt, ...) vcc::util::diagnostic_print(__FILE__, \
	__FUNCTION__, __LINE__, fmt, __VA_ARGS__);
#endif // __GNUC__

#define VKCHECK(expr) { vcc::vcc_exception::maybe_throw(expr, __FILE__, \
	__LINE__, __FUNCTION__, #expr); }
#define VKTRACE(expr) { expr; /*VCC_PRINT("%s", #expr)*/ }

#endif /* UTIL_H_ */
