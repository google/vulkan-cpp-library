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
#ifndef TYPE_TRANSFORM_H_
#define TYPE_TRANSFORM_H_

#include <algorithm>
#include <type/storage.h>
#include <type/supplier.h>

namespace type {

template<typename T>
class transform_type {
private:
	typedef storage_type<T, true, true> internal_storage_type;
	typedef mutable_storage_type<T, true> internal_mutable_storage_type;
	typedef std::function<void(internal_mutable_storage_type &&)> update_function_type;
	template<typename U>
	friend typename internal_storage_type::lock_type get_lock(transform_type<U> &array);

	template<typename U>
	friend revision_type &get_revision(transform_type<U> &array);

public:
	static const bool is_array = true;

	typedef typename internal_storage_type::iterator iterator;
	typedef typename internal_storage_type::const_iterator const_iterator;
	typedef typename internal_storage_type::size_type size_type;
	typedef typename internal_storage_type::value_type value_type;
	typedef typename internal_storage_type::reference reference;
	typedef typename internal_storage_type::const_reference const_reference;
	typedef typename internal_storage_type::pointer pointer;
	typedef typename internal_storage_type::const_pointer const_pointer;

	const_iterator begin() const {
		flush();
		return storage.cbegin();
	}

	const_iterator end() const {
		flush();
		return storage.cend();
	}

	const_iterator cbegin() const {
		flush();
		return storage.cbegin();
	}

	const_iterator cend() const {
		flush();
		return storage.cend();
	}

	const_reference operator[] (int index) const {
		flush();
		return storage[index];
	}

	size_type size() const {
		return storage.size();
	}

	template<typename ContainerT, typename FunctorT>
	transform_type(const supplier<ContainerT> &container, FunctorT functor)
		: update_function([container, functor](internal_mutable_storage_type &&storage) {
			std::transform(container->begin(), container->end(), storage.begin(), functor);
		  }),
		  storage(container->size()),
		  revision(REVISION_NONE) {}

private:
	void flush() const {
		update_function(mutate(storage));
	}

	update_function_type update_function;
	mutable internal_storage_type storage;
	revision_type revision;
};

template<typename ContainerT, typename FunctorT>
auto make_transform(ContainerT container, FunctorT functor)
		->transform_type<decltype(functor((*make_supplier(container))[0]))> {
	return transform_type<decltype(functor((*make_supplier(container))[0]))>(make_supplier(container), functor);
}

}  // namespace type

#endif // TYPE_TRANSFORM_H_
