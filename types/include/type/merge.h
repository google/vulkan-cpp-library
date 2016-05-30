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
#ifndef TYPE_MERGE_H_
#define TYPE_MERGE_H_

#include <type/revision.h>
#include <type/storage.h>
#include <type/supplier.h>

namespace type {

namespace internal {

template<typename T>
struct read_instance_type {
	explicit read_instance_type(std::size_t size, std::size_t container1_size)
		: size(size), container1_size(container1_size) {}
	virtual ~read_instance_type() {}
	virtual const T &get(std::size_t index) const = 0;
	const std::size_t size, container1_size;
};

template<typename T>
struct instance_type {
	explicit instance_type(std::size_t size, std::size_t container1_size)
		: size(size), container1_size(container1_size) {}
	virtual ~instance_type() {}
	virtual std::unique_ptr<read_instance_type<T>> read() const = 0;
	virtual revision_type revision() = 0;
	const std::size_t size, container1_size;
};

template<typename T, typename Container1T, typename Container2T>
struct read_merge_instance_type : public read_instance_type<T> {
	read_merge_instance_type(const supplier<Container1T> &container1,
		const supplier<Container2T> &container2)
		: read_instance_type(container1->size() + container2->size(),
			container1->size()),
		container1(container1), container2(container2) {}

	const T &get(std::size_t index) const {
		assert(index < size);
		return index < read_instance_type::container1_size
			? (*container1)[index]
			: (*container2)[index - read_instance_type::container1_size];
	}

	const supplier<Container1T> container1;
	const supplier<Container2T> container2;
};

template<typename T, typename Container1T, typename Container2T>
struct merge_instance : public instance_type<T> {

	merge_instance(const supplier<Container1T> &container1,
		const supplier<Container2T> &container2)
		: instance_type(container1->size() + container2->size(), container1->size()),
		container1(container1), container2(container2),
		revision1(REVISION_NONE), revision2(REVISION_NONE),
		this_revision(1) {}
	const supplier<Container1T> container1;
	const supplier<Container2T> container2;
	revision_type revision1, revision2, this_revision;

	std::unique_ptr<read_instance_type<T>> read() const {
		typedef decltype(type::read(*container1)) read_container1_type;
		typedef decltype(type::read(*container2)) read_container2_type;
		return std::unique_ptr<read_instance_type<T>>(
			new read_merge_instance_type<T, read_container1_type,
			read_container2_type>(type::read(*container1),
				type::read(*container2)));
	}

	revision_type revision() {
		const revision_type revision1(internal::get_revision(*container1)),
			revision2(internal::get_revision(*container2));
		if (revision1 > this->revision1 || revision2 > this->revision2) {
			++this_revision;
			this->revision1 = revision1;
			this->revision2 = revision2;
		}
		return this_revision;
	}
};

}  // namespace internal

template<typename T>
class merge_type {
	template<typename U>
	friend class read_merge_type;
public:

	static const bool is_array = true;

	typedef std::size_t size_type;
	typedef T value_type;
	typedef const value_type & reference;
	typedef reference const_reference;
	typedef const value_type * pointer;
	typedef pointer const_pointer;

	template<typename Container1T, typename Container2T>
	merge_type(const supplier<Container1T> &container1,
		const supplier<Container2T> &container2)
		: instance(new internal::merge_instance<T, Container1T, Container2T>(
			container1, container2)) {}

	size_type size() const {
		return instance->size;
	}

private:
	std::shared_ptr<internal::instance_type<T>> instance;
};

template<typename T>
class read_merge_type {

	template<typename U>
	friend read_merge_type<U> read(merge_type<U> &merge);
public:
	static const bool is_array = true;

	typedef std::size_t size_type;
	typedef T value_type;
	typedef const value_type & reference;
	typedef reference const_reference;
	typedef const value_type * pointer;
	typedef pointer const_pointer;

	struct iterator : public std::iterator<std::random_access_iterator_tag, T> {
		friend class read_merge_type;
	public:
		iterator() : merge(nullptr), index(0) {}
		iterator(const iterator&) = default;
		iterator(iterator &&i) : merge(i.merge), index(i.index) {
			i.merge = nullptr;
			i.index = 0;
		}
		iterator &operator=(const iterator &) = default;
		iterator &operator=(iterator &&i) {
			merge = i.merge;
			index = i.index;
			i.merge = nullptr;
			i.index = 0;
			return *this;
		}
		bool operator==(const iterator &i) const {
			return merge == i.merge && index == i.index;
		}
		bool operator!=(const iterator &i) const {
			return !this->operator ==(i);
		}
		const T &operator*() const {
			return (*merge)[index];
		}
		const T *operator->() const {
			return &(*merge)[index];
		}
		iterator &operator++() {
			assert(index < merge->size());
			++index;
			return *this;
		}
		iterator operator++(int) {
			const iterator it(*this);
			assert(index < merge->size);
			++index;
			return it;
		}
		iterator &operator--() {
			--index;
			assert(index >= 0);
			return *this;
		}
		iterator operator--(int) {
			const iterator it(*this);
			--index;
			assert(index >= 0);
			return it;
		}
		std::size_t operator+(const iterator &it) const {
			assert(merge == it.merge && index + it.index < merge->size);
			return index + it.index;
		}
		iterator operator+(int size) const {
			assert(index + size < merge->size());
			return iterator(merge, index + size);
		}
		std::size_t operator-(const iterator &it) const {
			assert(merge == it.merge && it.index <= index);
			return index - it.index;
		}
		iterator operator-(int size) const {
			assert(size <= index);
			return iterator(merge, index - size);
		}
		bool operator<(const iterator &it) const {
			assert(merge == it.merge);
			return index < it.index;
		}
		bool operator>(const iterator &it) const {
			assert(merge == it.merge);
			return index > it.index;
		}
		bool operator<=(const iterator &it) const {
			assert(merge == it.merge);
			return index <= it.index;
		}
		bool operator>=(const iterator &it) const {
			assert(merge == it.merge);
			return index >= it.index;
		}
		iterator &operator+=(int size) {
			assert(std::size_t(int(index) + size) < merge->size);
			index += size;
			return *this;
		}
		iterator &operator-=(int size) {
			assert(int(index) - size >= 0);
			index += size;
			return *this;
		}
		const T &operator[](int offset) const {
			assert(int(index) + offset >= 0 && std::size_t(int(index) + offset) < merge->size);
			return (*merge)[std::size_t(int(index) + offset)];
		}

	private:
		iterator(const read_merge_type<T> *merge, std::size_t index)
			: merge(merge), index(index) {}
		const read_merge_type<T> *merge;
		std::size_t index;
	};

	typedef iterator const_iterator;

	const_iterator begin() const {
		return iterator(this, 0);
	}

	const_iterator end() const {
		return iterator(this, instance->size);
	}

	const_iterator cbegin() const {
		return iterator(this, 0);
	}

	const_iterator cend() const {
		return iterator(this, instance->size());
	}

	const_reference operator[] (std::size_t index) const {
		return instance->get(index);
	}

	size_type size() const {
		return instance->size;
	}

private:
	explicit read_merge_type(const merge_type<T> &merge)
		: instance(merge.instance->read()) {}

	revision_type get_revision() const {
		return merge.instance->revision();
	}

	std::unique_ptr<internal::read_instance_type<T>> instance;
};

namespace internal {

template<typename ContainerT, typename Container1T, typename Container2T>
auto make_merge(Container1T container1, Container2T container2)
		->merge_type<typename ContainerT::value_type> {
	return merge_type<typename ContainerT::value_type>(
		type::make_supplier(container1), type::make_supplier(container2));
}

}  // namespace internal

template<typename Container1T, typename Container2T>
auto make_merge(Container1T container1, Container2T container2)->
		decltype(internal::make_merge<
			typename decltype(type::make_supplier(container1))::value_type,
			Container1T, Container2T>(
				std::forward<Container1T>(container1),
				std::forward<Container1T>(container2))) {
	return internal::make_merge<
			typename decltype(type::make_supplier(container1))::value_type,
			Container1T, Container2T>(
		std::forward<Container1T>(container1),
		std::forward<Container1T>(container2));
}

template<typename T>
read_merge_type<T> read(merge_type<T> &merge) {
	return read_merge_type<T>(merge);
}

}  // namespace type

#endif // TYPE_MERGE_H_
