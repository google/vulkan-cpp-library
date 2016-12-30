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
#include <array>
#include <functional>
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

	template<typename... Containers, typename Functor>
	static transform_type<T> make_transform(internal_storage_type &&storage, Functor functor,
		const supplier<Containers> &... container) {
		typedef std::array<revision_type, sizeof...(Containers)> revisions_type;
		revisions_type revisions;
		std::fill(std::begin(revisions), std::end(revisions), REVISION_NONE);
		return transform_type<T>(std::forward<internal_storage_type>(storage),
			std::bind([revisions, container...](Functor &functor,
				internal_writable_storage_type &&storage) mutable {
			// storage is locked in the scope of this function.
			revisions_type container_revisions{ internal::get_revision(*container)... };
			if (container_revisions != revisions) {
				functor(read(*container)..., std::forward<internal_writable_storage_type>(storage));
				revisions = container_revisions;
			}
		}, std::forward<Functor>(functor), std::placeholders::_1));
	}

	template<typename Function>
	transform_type(internal_storage_type &&storage, Function &&function)
		: update_function(function),
		  storage(std::forward<internal_storage_type>(storage)) {}

	size_type size() const {
		return storage.size();
	}

private:
	void flush() const {
		update_function(write(storage));
	}

	update_function_type update_function;
	mutable internal_storage_type storage;
};

template<typename T>
using read_transform_type = readable_t_array<T, true>;

template<typename T>
read_transform_type<T> read(transform_type<T> &array) {
	array.flush();
	return read(array.storage);
}

template<typename Storage, typename... Containers, typename FunctorT>
auto make_transform(Storage &&storage, FunctorT functor, Containers... container)
		->transform_type<typename Storage::value_type> {
	return transform_type<typename Storage::value_type>::make_transform(std::forward<Storage>(storage),
		std::forward<FunctorT>(functor), make_supplier(std::forward<Containers>(container))...);
}

}  // namespace type

#endif // TYPE_TRANSFORM_H_
