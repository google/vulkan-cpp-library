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

template<typename T>
class merge_type;

namespace internal {

template<typename T>
typename merge_type<T>::lock_type &get_lock(merge_type<T> &);

template<typename T>
revision_type &get_revision(merge_type<T> &);

}  // namespace internal

template<typename T>
class merge_type {
private:
	struct instance_type {
		explicit instance_type(std::size_t size, std::size_t container1_size)
			: size(size), container1_size(container1_size) {}
		virtual const T &get(std::size_t index) const = 0;
		virtual void lock() = 0;
		virtual void unlock() = 0;
		virtual revision_type revision() = 0;
		const std::size_t size, container1_size;
	};

	template<typename Container1T, typename Container2T>
	struct merge_instance : public instance_type {
		typedef typename Container1T::lock_type lock_type1;
		typedef typename Container2T::lock_type lock_type2;

		merge_instance(const supplier<Container1T> &container1, const supplier<Container2T> &container2)
			: instance_type(container1->size() + container2->size(), container1->size()),
			  container1(container1), container2(container2),
			  revision1(REVISION_NONE), revision2(REVISION_NONE), this_revision(1) {}
		const supplier<Container1T> container1;
		const supplier<Container2T> container2;
		lock_type1 container_lock1;
		lock_type2 container_lock2;
		revision_type revision1, revision2, this_revision;

		const T &get(std::size_t index) const {
			assert(index < size);
			return index < instance_type::container1_size
					? (*container1)[index] : (*container2)[index - instance_type::container1_size];
		}
		void lock() {
			container_lock1 = internal::get_lock(*container1);
			container_lock2 = internal::get_lock(*container2);
		}

		void unlock() {
			container_lock1 = lock_type1();
			container_lock2 = lock_type2();
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
public:
	struct lock_type {
		friend class merge_type;
	public:
		lock_type() = default;
		lock_type(const lock_type&) = delete;
		lock_type(lock_type &&c) : instance(c.instance) {
			c.instance = nullptr;
		}
		lock_type &operator=(const lock_type &) = delete;
		lock_type &operator=(lock_type &&c) {
			instance = c.instance;
			c.instance = nullptr;
			return *this;
		}
		~lock_type() {
			if (instance) {
				instance->unlock();
			}
		}
	private:
		lock_type(const std::shared_ptr<instance_type> &instance)
			: instance(instance) {
			instance->lock();
		}
		std::shared_ptr<instance_type> instance;
	};
	struct iterator : public std::iterator<std::random_access_iterator_tag, T> {
		friend class merge_type;
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
			assert(index++ < merge->size());
			return *this;
		}
		iterator operator++(int) {
			const iterator it(*this);
			assert(index++ < merge->size);
			return it;
		}
		iterator &operator--() {
			assert(--index >= 0);
			return *this;
		}
		iterator operator--(int) {
			const iterator it(*this);
			assert(--index >= 0);
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
		iterator(const merge_type<T> *merge, std::size_t index)
			: merge(merge), index(index) {}
		const merge_type<T> *merge;
		std::size_t index;
	};

	static const bool is_array = true;

	typedef iterator const_iterator;
	typedef std::size_t size_type;
	typedef T value_type;
	typedef const value_type & reference;
	typedef reference const_reference;
	typedef const value_type * pointer;
	typedef pointer const_pointer;

	template<typename Container1T, typename Container2T>
	merge_type(const supplier<Container1T> &container1, const supplier<Container2T> &container2)
		: instance(new merge_instance<Container1T, Container2T>(container1, container2)) {}

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

	const_reference operator[] (int index) const {
		return instance->get(index);
	}

	size_type size() const {
		return instance->size;
	}

private:
	lock_type create_lock() {
		return lock_type(instance);
	}
	std::shared_ptr<instance_type> instance;
};

template<typename Container1T, typename Container2T>
auto make_merge(Container1T container1, Container2T container2)
		/*->merge_type<typename decltype(type::make_supplier(container1))::value_type::value_type>*/ {
	return merge_type<typename decltype(type::make_supplier(container1))::value_type::value_type>(
		type::make_supplier(container1), type::make_supplier(container2));
}

namespace internal {

template<typename T>
typename merge_type<T>::lock_type &get_lock(merge_type<T> &merge) {
	return merge.create_lock();
}

template<typename T>
revision_type &get_revision(merge_type<T> &merge) {
	return merge.instance->revision();
}

}  // namespace internal

}  // namespace type

#endif // TYPE_MERGE_H_
