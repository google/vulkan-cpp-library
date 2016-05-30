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
#ifndef GTYPE_ARRAY_TYPE_H_
#define GTYPE_ARRAY_TYPE_H_

#include <type/internal.h>
#include <type/revision.h>
#include <mutex>
#include <vector>
#include <iostream>

namespace type {

namespace internal {

template<typename T, bool Mutable, bool IsArray = true>
class storage_type {
	template<typename U>
	friend auto type::internal::get_revision(U &v)
		->decltype(v.get_revision())&;

	template<typename U>
	friend auto type::internal::get_container(U &v)
		->decltype(v.get_container())&;

	template<typename U>
	friend auto type::internal::get_lock(U &v)->decltype(v.get_lock())&;

	typedef std::vector<T> container_type;
public:
	typedef std::mutex mutex_type;
	typedef std::unique_lock<mutex_type> lock_type;

	static const bool is_array = IsArray;

	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename container_type::reference reference;
	typedef typename container_type::const_reference const_reference;
	typedef typename container_type::pointer pointer;
	typedef typename container_type::const_pointer const_pointer;

	explicit storage_type(std::size_t size, const T &value = T())
		: array(size, value), revision(1) {}

	template<typename IteratorT>
	storage_type(IteratorT begin, IteratorT end)
		: array(begin, end), revision(1) {}

	storage_type(std::initializer_list<value_type> &&initializer)
		: array(std::forward<std::initializer_list<value_type>>(initializer)),
		  revision(1) {}

	template<bool _Mutable, bool _IsArray>
	storage_type(const storage_type<T, _Mutable, _IsArray> &c)
		: storage_type(c.internal_copy()) {}

	// Not thread-safe for obvious reasons.
	// If multiple threads have access, then you used std::move,
	// which should be considered non thread-safe.
	// Note: Old object will be in an invalid state (its size will be zero)
	template<bool _Mutable, bool _IsArray>
	storage_type(storage_type<T, _Mutable, _IsArray> &&c)
		: array(std::move(internal::get_container(c))),
		  revision(internal::get_revision(c)) {}

	// Provided only since compiler fails to see above copy constructor even
	// with _Mutable = Mutable.
	// Note: Old object will be in an invalid state (its size will be zero)
	storage_type(const storage_type<T, Mutable, IsArray> &c)
		: storage_type(c.internal_copy()) {}
	storage_type(storage_type<T, Mutable, IsArray> &&) = default;

	size_type size() const {
		return array.size();
	}

protected:

	explicit storage_type(std::tuple<container_type, revision_type> &&copy)
		: array(std::forward<container_type>(std::get<0>(copy))),
		  revision(std::get<1>(copy)) {}

	std::tuple<container_type, revision_type> internal_copy() const {
		std::lock_guard<std::mutex> lock(this->lock);
		return std::make_tuple(array, revision);
	}

	container_type array;
	mutable std::mutex lock;
	revision_type revision;

private:
	container_type &get_container() {
		return array;
	}

	std::mutex &get_lock() {
		return lock;
	}

	revision_type &get_revision() {
		return revision;
	}
};

}  // end namespace internal

template<typename T, bool Mutable, bool IsArray> class storage_type;

template<typename T, bool Mutable>
class storage_type<T, Mutable, true>
		: public internal::storage_type<T, Mutable, true> {
	template<typename U, bool _IsArray>
	friend class writable_storage_type;

public:
	explicit storage_type(std::size_t size, const T &value = T())
		: internal::storage_type<T, Mutable, true>(size, value) {}

	template<typename IteratorT>
	storage_type(IteratorT begin, IteratorT end)
		: internal::storage_type<T, Mutable, true>(begin, end) {}

	storage_type(std::initializer_list<T> &&initializer)
		: internal::storage_type<T, Mutable, true>(
			std::forward<std::initializer_list<T>>(initializer)) {}

	template<bool _Mutable, bool _IsArray>
	storage_type(const storage_type<T, _Mutable, _IsArray> &c)
		: storage_type(c.internal_copy()) {}

	template<bool _Mutable, bool _IsArray>
	storage_type(storage_type<T, _Mutable, _IsArray> &&c)
		: internal::storage_type<T, Mutable, true>(
			std::forward<storage_type<T, _Mutable, _IsArray>>(c)) {}

	storage_type(const storage_type<T, Mutable, true> &c)
		: internal::storage_type<T, Mutable, true>(c.internal_copy()) {}
	storage_type(storage_type<T, Mutable, true> &&) = default;
};

template<typename T, bool Mutable>
class storage_type<T, Mutable, false>
	: public internal::storage_type<T, Mutable, false> {
public:

	explicit storage_type(const T &value = T())
		: internal::storage_type<T, Mutable, false>(1, value) {}

	template<bool _Mutable>
	storage_type(const storage_type<T, _Mutable, false> &c)
		: internal::storage_type<T, Mutable, false>(c) {}

	template<bool _Mutable>
	storage_type(storage_type<T, _Mutable, false> &&c)
		: internal::storage_type<T, Mutable, false>(
				std::forward<storage_type<T, _Mutable, false>>(c)) {}

	storage_type(const storage_type<T, Mutable, false> &c) = default;
	storage_type(storage_type<T, Mutable, false> &&) = default;
};

namespace internal {

template<typename T, bool Mutable, bool IsArray>
class readable_storage_type {
protected:
	typedef storage_type<T, Mutable, IsArray> target_type;

	explicit readable_storage_type(target_type &array)
		: lock(internal::get_lock(array)), array(&array) {}

public:
	static const bool is_array = IsArray;

	typedef typename target_type::iterator iterator;
	typedef typename target_type::const_iterator const_iterator;
	typedef typename target_type::size_type size_type;
	typedef typename target_type::value_type value_type;
	typedef typename target_type::reference reference;
	typedef typename target_type::const_reference const_reference;
	typedef typename target_type::pointer pointer;
	typedef typename target_type::const_pointer const_pointer;

	readable_storage_type() : array(nullptr) {}
	readable_storage_type(const readable_storage_type<T, Mutable, IsArray> &) = delete;
	readable_storage_type(readable_storage_type<T, Mutable, IsArray> &&) = default;
	readable_storage_type &operator=(
		const readable_storage_type<T, Mutable, IsArray> &) = delete;
	readable_storage_type &operator=(
		readable_storage_type<T, Mutable, IsArray> &&copy) {
		lock = std::move(copy.lock);
		array = copy.array;
		copy.array = nullptr;
		return *this;
	}
	~readable_storage_type() {
		if (array) {
			++internal::get_revision(*array);
		}
	}

	const_iterator begin() const {
		return internal::get_container(*array).cbegin();
	}

	const_iterator end() const {
		return internal::get_container(*array).cend();
	}

	const_reference operator[] (std::size_t index) const {
		return internal::get_container(*array)[index];
	}

	size_type size() const {
		return internal::get_container(*array).size();
	}

private:
	std::unique_lock<std::mutex> lock;
	target_type *array;
};

}  // namespace internal

template<typename T, bool Mutable, bool IsArray>
class readable_storage_type;

template<typename T, bool Mutable>
class readable_storage_type<T, Mutable, true>
	: public internal::readable_storage_type<T, Mutable, true> {

	typedef typename internal::readable_storage_type<T, Mutable, true>
		::target_type target_type;

	template<typename U, bool _Mutable, bool _IsArray>
	friend readable_storage_type<U, _Mutable, _IsArray> read(
		storage_type<U, _Mutable, _IsArray> &array);
private:
	explicit readable_storage_type(target_type &array)
		: internal::readable_storage_type<T, Mutable, true>(array) {}

public:
	typedef typename internal::readable_storage_type<T, Mutable, true>
		::const_reference const_reference;

	readable_storage_type() = default;
	readable_storage_type(
		const readable_storage_type<T, Mutable, true> &) = delete;
	readable_storage_type(
		readable_storage_type<T, Mutable, true> &&) = default;
	readable_storage_type &operator=(
		const readable_storage_type<T, Mutable, true> &) = delete;
	readable_storage_type &operator=(
		readable_storage_type<T, Mutable, true> &&copy) = default;
};

template<typename T, bool Mutable>
class readable_storage_type<T, Mutable, false>
		: public internal::readable_storage_type<T, Mutable, false> {

	typedef typename internal::readable_storage_type<T, Mutable, false>
		::target_type target_type;

	template<typename U, bool _Mutable, bool _IsArray>
	friend readable_storage_type<U, _Mutable, _IsArray> read(
		storage_type<U, _Mutable, _IsArray> &array);

private:
	explicit readable_storage_type(target_type &array)
		: internal::readable_storage_type<T, Mutable, false>(array) {}

public:
	typedef typename internal::readable_storage_type<T, Mutable, false>
		::const_reference const_reference;

	readable_storage_type() = default;
	readable_storage_type(
		const readable_storage_type<T, Mutable, false> &) = delete;
	readable_storage_type(
		readable_storage_type<T, Mutable, false> &&) = default;
	readable_storage_type &operator=(
		const readable_storage_type<T, Mutable, false> &) = delete;
	readable_storage_type &operator=(
		readable_storage_type<T, Mutable, false> &&copy) = default;

	operator const_reference() const {
		return (*this)[0];
	}

	bool operator==(const T &value) const {
		return (*this)[0] == value;
	}
};

template<typename T, bool Mutable, bool IsArray>
readable_storage_type<T, Mutable, IsArray> read(
	storage_type<T, Mutable, IsArray> &array) {
	return readable_storage_type<T, Mutable, IsArray>(array);
}

template<typename T, bool IsArray>
class writable_storage_type {
	template<typename U, bool _IsArray>
	friend writable_storage_type<U, _IsArray> write(
		storage_type<U, true, _IsArray> &array);
private:
	typedef storage_type<T, true, IsArray> target_type;

	explicit writable_storage_type(target_type &array)
		: lock(internal::get_lock(array)), array(&array) {}

public:
	static const bool is_array = IsArray;

	typedef typename target_type::iterator iterator;
	typedef typename target_type::const_iterator const_iterator;
	typedef typename target_type::size_type size_type;
	typedef typename target_type::value_type value_type;
	typedef typename target_type::reference reference;
	typedef typename target_type::const_reference const_reference;
	typedef typename target_type::const_pointer const_pointer;
	typedef typename target_type::pointer pointer;

	writable_storage_type() : array(nullptr) {}
	writable_storage_type(const writable_storage_type<T, IsArray> &) = delete;
	writable_storage_type(writable_storage_type<T, IsArray> &&) = default;
	writable_storage_type &operator=(
		const writable_storage_type<T, IsArray> &) = delete;
	writable_storage_type &operator=(
			writable_storage_type<T, IsArray> &&copy) {
		lock = std::move(copy.lock);
		array = copy.array;
		copy.array = nullptr;
		return *this;
	}
	~writable_storage_type() {
		if (array) {
			++internal::get_revision(*array);
		}
	}

	iterator begin() const {
		return internal::get_container(*array).begin();
	}

	iterator end() const {
		return internal::get_container(*array).end();
	}

	reference operator[] (std::size_t index) const {
		return internal::get_container(*array)[index];
	}

	size_type size() const {
		return internal::get_container(*array).size();
	}

private:
	std::unique_lock<std::mutex> lock;
	target_type *array;
};

template<typename T, bool IsArray>
writable_storage_type<T, IsArray> write(
		storage_type<T, true, IsArray> &array) {
	return writable_storage_type<T, IsArray>(array);
}

template<typename T>
using const_t_array = storage_type<T, false, true>;
template<typename T>
using t_array = storage_type<T, true, true>;

template<typename T>
using const_t_primitive = storage_type<T, false, false>;
template<typename T>
using t_primitive = storage_type<T, true, false>;

template<typename T, bool Mutable>
using readable_t_array = readable_storage_type<T, Mutable, true>;
template<typename T, bool Mutable>
using readable_t_primitive = readable_storage_type<T, Mutable, false>;

template<typename T>
using writable_t_array = writable_storage_type<T, true>;
template<typename T>
using writable_t_primitive = writable_storage_type<T, false>;

}  // namespace type

#endif // GTYPE_ARRAY_TYPE_H_
