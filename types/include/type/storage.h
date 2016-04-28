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

#include <type/revision.h>
#include <mutex>
#include <vector>

namespace type {

template<typename T, bool Mutable, bool IsArray> class storage_type;

namespace internal {

template<typename T, bool Mutable, bool IsArray> class storage_type;

template<typename T, bool Mutable, bool IsArray>
typename storage_type<T, Mutable, IsArray>::container_type &get_container(storage_type<T, Mutable, IsArray> &array);

template<typename T, bool Mutable, bool IsArray>
typename storage_type<T, Mutable, IsArray>::lock_type get_lock(storage_type<T, Mutable, IsArray> &array);

template<typename T, bool Mutable, bool IsArray>
revision_type &get_revision(storage_type<T, Mutable, IsArray> &array);

template<typename T, bool Mutable, bool IsArray = true>
class storage_type {
public:
	typedef std::mutex mutex_type;
	typedef std::unique_lock<mutex_type> lock_type;
	typedef std::vector<T> container_type; // TODO: make private
private:
	template<typename U, bool _Mutable, bool _IsArray>
	friend typename storage_type<U, _Mutable, _IsArray>::container_type &get_container(storage_type<U, _Mutable, _IsArray> &array);

	template<typename U, bool _Mutable, bool _IsArray>
	friend typename storage_type<U, _Mutable, _IsArray>::lock_type get_lock(storage_type<U, _Mutable, _IsArray> &array);

	template<typename U, bool _Mutable, bool _IsArray>
	friend revision_type &get_revision(storage_type<U, _Mutable, _IsArray> &array);

public:
	static const bool is_array = IsArray;

	typedef typename container_type::const_iterator iterator;
	typedef iterator const_iterator;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename container_type::const_reference reference;
	typedef reference const_reference;
	typedef typename container_type::const_pointer pointer;
	typedef pointer const_pointer;

	explicit storage_type(std::size_t size, const T &value = T()) : array(size, value), revision(1) {}

	template<typename IteratorT>
	storage_type(IteratorT begin, IteratorT end) : array(begin, end), revision(1) {}

	storage_type(std::initializer_list<value_type> &&initializer)
		: array(std::forward<std::initializer_list<value_type>>(initializer)),
		  revision(1) {}

	template<bool _Mutable, bool _IsArray>
	storage_type(const storage_type<T, _Mutable, _IsArray> &c)
		: storage_type(c.internal_copy()) {}

	// Not thread-safe for obvious reasons.
	// If multiple threads have access, then you used std::move, which should be considered non thread-safe.
	// Note: Old object will be in an invalid state (its size will be zero)
	template<bool _Mutable, bool _IsArray>
	storage_type(storage_type<T, _Mutable, _IsArray> &&c)
		: array(std::move(internal::get_container(c))),
		  revision(internal::get_revision(c)) {}

	// Provided only since compiler fails to see above copy constructor even with _Mutable = Mutable.
	// Note: Old object will be in an invalid state (its size will be zero)
	storage_type(const storage_type<T, Mutable, IsArray> &c)
		: storage_type(c.internal_copy()) {}
	storage_type(storage_type<T, Mutable, IsArray> &&) = default;

	const_iterator begin() const {
		return array.cbegin();
	}

	const_iterator end() const {
		return array.cend();
	}

	const_iterator cbegin() const {
		return array.cbegin();
	}

	const_iterator cend() const {
		return array.cend();
	}

	const_reference operator[] (std::size_t index) const {
		return array[index];
	}

	size_type size() const {
		return array.size();
	}

private:

	explicit storage_type(std::tuple<container_type, revision_type> &&copy)
		: array(std::forward<container_type>(std::get<0>(copy))), revision(std::get<1>(copy)) {}

	std::tuple<container_type, revision_type> internal_copy() const {
		std::lock_guard<std::mutex> lock(this->lock);
		return std::make_tuple(array, revision);
	}

	container_type array;
	mutable std::mutex lock;
	revision_type revision;
};

template<typename T, bool Mutable, bool IsArray>
typename storage_type<T, Mutable, IsArray>::container_type &get_container(
		storage_type<T, Mutable, IsArray> &array) {
	return array.array;
}

template<typename T, bool Mutable, bool IsArray>
typename storage_type<T, Mutable, IsArray>::lock_type get_lock(storage_type<T, Mutable, IsArray> &array) {
	return typename storage_type<T, Mutable, IsArray>::lock_type(array.lock);
}

template<typename T, bool Mutable, bool IsArray>
revision_type &get_revision(storage_type<T, Mutable, IsArray> &array) {
	return array.revision;
}

}  // end namespace internal

template<typename T, bool Mutable, bool IsArray>
class storage_type;

template<typename T, bool Mutable>
class storage_type<T, Mutable, true> : public internal::storage_type<T, Mutable, true> {
	template<typename U, bool _IsArray>
	friend class mutable_storage_type;
private:
	typedef typename internal::storage_type<T, Mutable, true>::container_type container_type;
public:

	explicit storage_type(std::size_t size, const T &value = T())
		: internal::storage_type<T, Mutable, true>(size, value) {}

	template<typename IteratorT>
	storage_type(IteratorT begin, IteratorT end)
		: internal::storage_type<T, Mutable, true>(begin, end) {}

	storage_type(std::initializer_list<T> &&initializer)
		: internal::storage_type<T, Mutable, true>(std::forward<std::initializer_list<T>>(initializer)) {}

	template<bool _Mutable, bool _IsArray>
	storage_type(const storage_type<T, _Mutable, _IsArray> &c)
		: storage_type(c.internal_copy()) {}

	template<bool _Mutable, bool _IsArray>
	storage_type(storage_type<T, _Mutable, _IsArray> &&c)
		: internal::storage_type<T, Mutable, true>(std::forward<storage_type<T, _Mutable, _IsArray>>(c)) {}

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

	operator const T &() const {
		return (*this)[0];
	}

	bool operator==(const T &value) const {
		return (*this)[0] == value;
	}
};

template<typename T, bool IsArray>
class mutable_storage_type {
	template<typename U, bool _IsArray>
	friend mutable_storage_type<U, _IsArray> mutate(storage_type<U, true, _IsArray> &array);
private:
	typedef storage_type<T, true, IsArray> target_type;

	explicit mutable_storage_type(target_type &array)
		: lock(internal::get_lock(array)), array(&array) {}

public:
	static const bool is_array = IsArray;

	typedef typename target_type::container_type::iterator iterator;
	typedef typename target_type::const_iterator const_iterator;
	typedef typename target_type::size_type size_type;
	typedef typename target_type::value_type value_type;
	typedef typename target_type::container_type::reference reference;
	typedef typename target_type::const_reference const_reference;
	typedef typename target_type::const_pointer const_pointer;
	typedef typename target_type::container_type::pointer pointer;

	mutable_storage_type() : array(nullptr) {}
	mutable_storage_type(const mutable_storage_type<T, IsArray> &) = delete;
	mutable_storage_type(mutable_storage_type<T, IsArray> &&) = default;
	mutable_storage_type &operator=(const mutable_storage_type<T, IsArray> &) = delete;
	mutable_storage_type &operator=(mutable_storage_type<T, IsArray> &&copy) {
		lock = std::move(copy.lock);
		array = copy.array;
		copy.array = nullptr;
		return *this;
	}
	~mutable_storage_type() {
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
mutable_storage_type<T, IsArray> mutate(storage_type<T, true, IsArray> &array) {
	return mutable_storage_type<T, IsArray>(array);
}

template<typename T>
using const_t_array = storage_type<T, false, true>;
template<typename T>
using t_array = storage_type<T, true, true>;

template<typename T>
using const_t_primitive = storage_type<T, false, false>;
template<typename T>
using t_primitive = storage_type<T, true, false>;

template<typename T>
using mutable_t_array = mutable_storage_type<T, true>;
template<typename T>
using mutable_t_primitive = mutable_storage_type<T, false>;

}  // namespace type

#endif // GTYPE_ARRAY_TYPE_H_
