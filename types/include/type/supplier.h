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
#ifndef TYPE_SUPPLIER_H_
#define TYPE_SUPPLIER_H_

#include <functional>
#include <memory>

namespace type {

template<typename T>
class supplier {
private:
	typedef std::function<T&()> function_type;
public:
	typedef T value_type;

	supplier() = default;
	supplier(const supplier &copy) = default;
	supplier(supplier &&) = default;
	supplier &operator=(const supplier &) = default;
	supplier &operator=(supplier &&copy) = default;

	bool operator==(std::nullptr_t) const {
		return function;
	}

	T &get() const {
		return function();
	}

	T &operator*() const {
		return get();
	}

	T *operator->() const {
		return &get();
	}

	operator bool() const {
		return (bool) function;
	}

	template<typename... Args>
	auto operator() (Args... values) const
			->decltype(this->get()(std::forward<Args>(values)...)) {
		return get()(std::forward<Args>(values)...);
	}

	supplier(function_type &&functor)
		: function(std::forward<function_type>(functor)) {}

	supplier(const std::shared_ptr<T> &supplier)
		: function([supplier]()->T& {return *supplier;}) {}

	supplier(std::unique_ptr<T> &&s)
		: supplier(std::shared_ptr<T>(std::forward<std::unique_ptr<T>>(s))) {}

	supplier(const std::reference_wrapper<T> &reference)
		: function([reference]()->T& {return reference.get();}) {}

	supplier(T &&instance)
		: supplier<T>(std::make_shared<T>(std::forward<T>(instance))) {}

private:
	function_type function;
};

namespace internal {

template<typename T>
struct supplier_lookup_type {
	typedef typename std::remove_reference<T>::type type;
};

template<typename T>
struct supplier_lookup_type<std::shared_ptr<T>> {
	typedef T type;
};
template<typename T>
struct supplier_lookup_type<const std::shared_ptr<T>> {
	typedef T type;
};

template<typename T>
struct supplier_lookup_type<std::unique_ptr<T>> {
	typedef T type;
};

template<typename T>
struct supplier_lookup_type<std::reference_wrapper<T>> {
	typedef T type;
};

template<typename T>
struct supplier_lookup_type<const std::reference_wrapper<T>> {
	typedef T type;
};

}  // namespace internal

template<typename T>
supplier<typename internal::supplier_lookup_type<T>::type> make_supplier(
		const T &instance) {
	return supplier<typename internal::supplier_lookup_type<T>::type>(
		instance);
}
template<typename T>
supplier<typename internal::supplier_lookup_type<T>::type> make_supplier(
		T &instance) {
	return supplier<typename internal::supplier_lookup_type<T>::type>(
		instance);
}
template<typename T>
supplier<typename internal::supplier_lookup_type<T>::type> make_supplier(
		T &&instance) {
	return supplier<typename internal::supplier_lookup_type<T>::type>(
		std::forward<T>(instance));
}

}

#endif // TYPE_SUPPLIER_H_
