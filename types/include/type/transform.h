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
	typedef t_array<T> internal_storage_type;
	typedef writable_storage_type<T, true> internal_writable_storage_type;
	typedef std::function<void(internal_writable_storage_type &&)> update_function_type;

	template<typename U>
	friend readable_t_array<U, true> read(transform_type<U> &);

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

	template<typename ContainerT, typename FunctorT>
	transform_type(const supplier<ContainerT> &container, FunctorT functor)
		: update_function([container, functor](internal_writable_storage_type &&storage) {
			auto read_container(read(*container));
			std::transform(read_container.begin(), read_container.end(),
				storage.begin(), functor);
		  }),
		  storage(container->size()),
		  revision(REVISION_NONE) {}

	size_type size() const {
		return storage.size();
	}

private:
	void flush() const {
		update_function(write(storage));
	}

	update_function_type update_function;
	mutable internal_storage_type storage;
	revision_type revision;
};

template<typename T>
using read_transform_type = readable_t_array<T, true>;

template<typename T>
read_transform_type<T> read(transform_type<T> &array) {
	array.flush();
	return read(array.storage);
}

template<typename ContainerT, typename FunctorT>
auto make_transform(ContainerT container, FunctorT functor)->
		transform_type<decltype(functor(read(*make_supplier(container))[0]))> {
	typedef decltype(functor(read(*make_supplier(container))[0])) type;
	return transform_type<type>(make_supplier(container), functor);
}

}  // namespace type

#endif // TYPE_TRANSFORM_H_
