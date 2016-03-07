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

#include <memory>
#include <set>
#include <stdexcept>
#include <type/supplier.h>
#include <vector>

#if defined(_WIN32) && !defined(VK_USE_PLATFORM_WIN32_KHR)
#define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32
#define VK_PROTOTYPES 1
#include <vulkan/vulkan.h>

#ifdef _WIN32
#ifdef _USRDLL
#define VCC_LIBRARY __declspec ( dllexport )
#else
#define VCC_LIBRARY __declspec ( dllimport )
#endif // _USRDLL
#else
#define VCC_LIBRARY
#endif // _WIN32

namespace vcc {

// TODO(gardell): Rename to exception, perhaps add more types of exceptions.
class vcc_exception : public std::logic_error {
public:
	vcc_exception(const char *what) : std::logic_error(what) {}
	vcc_exception(const std::string &what) : std::logic_error(what) {}

	VCC_LIBRARY static void maybe_throw(VkResult result, const char *file, int line, const char *function, const char *expr);
};

namespace util {

VCC_LIBRARY std::vector<const char *> to_pointers(const std::vector<std::string> &vector);
VCC_LIBRARY std::vector<const char *> to_pointers(const std::set<std::string> &set);

template<typename T, typename... ValuesT>
std::vector<T> vector_from_variadic_movables(ValuesT &&...values) {
	// Can't use initializer-list in these cases.
	std::vector<T> vector;
	vector.reserve(sizeof...(ValuesT));
	// dummy integer array guarantees order of evaluation on early GCC versions (bug).
	const int dummy[] = { (vector.push_back(std::forward<ValuesT>(values)), 0)... };
	return std::move(vector);
}

VCC_LIBRARY void diagnostic_print(const char *filename, const char *function, int line, const char *fmt, ...);

namespace internal {

struct pass {
	template<typename ...T> pass(T...) {}
};

}  // namespace internal

}  // namespace util
}  // namespace vcc

#define VCC_PRINT(fmt, ...) vcc::util::diagnostic_print(__FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__);

#define VKCHECK(expr) { vcc::vcc_exception::maybe_throw(expr, __FILE__, __LINE__, __FUNCTION__, #expr); }
#ifdef WIN32
#define VKTRACE(expr) { /*OutputDebugString(#expr "\n");*/ expr; }
#else
#define VKTRACE(expr) { std::cerr << #expr << std::endl; expr; }
#endif // !WIN32

#endif /* UTIL_H_ */
