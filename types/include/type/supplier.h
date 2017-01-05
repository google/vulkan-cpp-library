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

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>

namespace type {

namespace internal {

template<typename T>
class supplier_impl {
	template<typename U>
	friend class supplier_impl;
public:
	typedef T value_type;

	supplier_impl() : pointer(nullptr) {}
	supplier_impl(const supplier_impl<T> &copy)
		: pointer(copy.pointer), shared_reference(copy.shared_reference) {}
	supplier_impl(supplier_impl<T> &&copy)
		: pointer(copy.pointer), shared_reference(std::move(copy.shared_reference)) {
		copy.pointer = nullptr;
	}
	template<typename U>
	supplier_impl(const supplier_impl<U> &copy)
		: pointer(copy.pointer), shared_reference(copy.shared_reference) {}
	template<typename U>
	supplier_impl(supplier_impl<U> &&copy)
		: pointer(copy.pointer), shared_reference(std::move(copy.shared_reference)) {
		copy.pointer = nullptr;
	}

	supplier_impl &operator=(const supplier_impl &copy) {
		pointer = copy.pointer;
		shared_reference = copy.shared_reference;
		return *this;
	}

	supplier_impl &operator=(supplier_impl &&copy) {
		pointer = copy.pointer;
		copy.pointer = nullptr;
		shared_reference = std::move(copy.shared_reference);
		return *this;
	}

	template<typename U>
	supplier_impl<T> &operator=(const supplier_impl<U> &copy) {
		pointer = copy.pointer;
		shared_reference = copy.shared_reference;
		return *this;
	}

	template<typename U>
	supplier_impl<T> &operator=(supplier_impl<U> &&copy) {
		pointer = copy.pointer;
		copy.pointer = nullptr;
		shared_reference = std::move(copy.shared_reference);
		return *this;
	}

	T &get() const {
		assert(pointer);
		return *pointer;
	}

	T &operator*() const {
		return get();
	}

	T *operator->() const {
		return &get();
	}

	operator bool() const {
		return !!pointer;
	}

	template<typename... Args>
	auto operator() (Args... values) const
			->decltype(std::declval<T&>()(std::forward<Args>(values)...)) {
		return get()(std::forward<Args>(values)...);
	}

	template<typename U>
	supplier_impl(const std::shared_ptr<U> &supplier)
		: pointer(supplier.get()), shared_reference(supplier) {}

	template<typename U>
	supplier_impl(std::unique_ptr<U> &&s)
		: supplier_impl(std::shared_ptr<T>(std::forward<std::unique_ptr<T>>(s))) {}

	supplier_impl(const std::reference_wrapper<T> &reference) : pointer(&reference.get()) {}
	template<typename U>
	supplier_impl(const std::reference_wrapper<U> &reference) : pointer(&reference.get()) {}

	supplier_impl(typename std::remove_const<T>::type &&instance)
		: supplier_impl(std::make_shared<T>(
			std::forward<typename std::remove_const<T>::type>(instance))) {}

private:
	T *pointer;
	std::shared_ptr<T> shared_reference;
};

} // namespace internal

template<typename T>
struct supplier : internal::supplier_impl<T> {

	supplier() = default;
	supplier(const supplier<T> &copy) = default;
	supplier(supplier<T> &&copy) = default;
	supplier(const std::shared_ptr<T> &supplier)
		: internal::supplier_impl<T>(supplier) {}
	supplier(std::unique_ptr<T> &&s)
		: internal::supplier_impl<T>(std::forward<std::unique_ptr<T>>(s)) {}
	supplier(const std::reference_wrapper<T> &reference)
		: internal::supplier_impl<T>(reference) {}
	supplier(T &&instance) : internal::supplier_impl<T>(
		std::forward<typename std::remove_const<T>::type>(instance)) {}

	supplier &operator=(const supplier &) = default;
	supplier &operator=(supplier &&) = default;
};

template<typename T>
struct supplier<const T> : internal::supplier_impl<const T> {

	supplier() = default;
	supplier(const supplier<const T> &copy) = default;
	supplier(supplier<const T> &&copy) = default;
	supplier(const supplier<T> &copy) : internal::supplier_impl<const T>(copy) {}
	supplier(supplier<T> &&copy)
		: internal::supplier_impl<const T>(std::forward<supplier<T>>(copy)) {}
	supplier(const std::shared_ptr<const T> &supplier)
		: internal::supplier_impl<const T>(supplier) {}
	supplier(const std::shared_ptr<T> &supplier) : internal::supplier_impl<const T>(supplier) {}
	supplier(std::unique_ptr<const T> &&s)
		: internal::supplier_impl<const T>(std::forward<std::unique_ptr<T>>(s)) {}
	supplier(std::unique_ptr<T> &&s)
		: internal::supplier_impl<const T>(std::forward<std::unique_ptr<T>>(s)) {}
	supplier(const std::reference_wrapper<const T> &reference)
		: internal::supplier_impl<const T>(reference) {}
	supplier(const std::reference_wrapper<T> &reference)
		: internal::supplier_impl<const T>(reference) {}
	supplier(T &&instance) : internal::supplier_impl<const T>(
		std::forward<typename std::remove_const<T>::type>(instance)) {}

	supplier &operator=(const supplier &) = default;
	supplier &operator=(supplier &&) = default;
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
	return supplier<typename internal::supplier_lookup_type<T>::type>(instance);
}
template<typename T>
supplier<typename internal::supplier_lookup_type<T>::type> make_supplier(
		T &instance) {
	return supplier<typename internal::supplier_lookup_type<T>::type>(instance);
}
template<typename T>
supplier<typename internal::supplier_lookup_type<T>::type> make_supplier(
		T &&instance) {
	return supplier<typename internal::supplier_lookup_type<T>::type>(std::forward<T>(instance));
}

}

#endif // TYPE_SUPPLIER_H_
