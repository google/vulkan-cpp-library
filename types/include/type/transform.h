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

namespace internal {

template<std::size_t I>
struct transform_get_revisions_type {
	template<typename Container>
	static std::array<revision_type, std::tuple_size<Container>::value> get_revisions(
			const Container &container) {
		std::array<revision_type, std::tuple_size<Container>::value> revisions(
			transform_get_revisions_type<I - 1>::get_revisions(container));
		std::get<I - 1>(revisions) = get_revision(*std::get<I - 1>(container));
		return revisions;
	}
};

template<>
struct transform_get_revisions_type<0> {
	template<typename Container>
	static std::array<revision_type, std::tuple_size<Container>::value> get_revisions(const Container &) {
		return std::array<revision_type, std::tuple_size<Container>::value>();
	}
};

template<std::size_t I>
struct transform_read_type {
	template<typename Functor, typename Storage, typename Container, typename... Readers>
	static void read(Functor &functor, Storage &&storage, Container &container, const Readers&... readers) {
		transform_read_type<I - 1>::read(functor, std::forward<Storage>(storage), container,
			std::get<I - 1>(container), readers...);
	}
};

template<>
struct transform_read_type<0> {
	template<typename Functor, typename Storage, typename Container, typename... Readers>
	static void read(Functor &functor, Storage &&storage, Container &container,
			const Readers&... readers) {
		functor(type::read(*readers)..., std::forward<Storage>(storage));
	}
};

template<typename T, bool IsArray>
struct transform_type_impl {
	typedef writable_storage_type<T, IsArray> internal_writable_storage_type;
	virtual void update(internal_writable_storage_type &&) = 0;
	virtual revision_type &get_revision() = 0;
};

template<typename T, bool IsArray, typename Functor, typename... Containers>
struct template_transform_type_impl : transform_type_impl<T, IsArray> {
	typedef writable_storage_type<T, IsArray> internal_writable_storage_type;
	typedef std::array<revision_type, sizeof...(Containers)> revisions_type;

	template_transform_type_impl(Functor functor, const supplier<Containers> &... container)
			: functor(std::forward<Functor>(functor)), revision(1)
			, container(std::make_tuple(container...)) {
		std::fill(std::begin(revisions), std::end(revisions), REVISION_NONE);
	}

	void update(internal_writable_storage_type &&storage) override {
		// storage is locked in the scope of this function.
		revisions_type current_revisions(
			transform_get_revisions_type<sizeof...(Containers)>::get_revisions(container));
		if (current_revisions > revisions) {
			transform_read_type<sizeof...(Containers)>::read(functor,
				std::forward<internal_writable_storage_type>(storage), container);
			revisions = current_revisions;
		}
	}
	revision_type &get_revision() override {
		revisions_type current_revisions(
			transform_get_revisions_type<sizeof...(Containers)>::get_revisions(container));
		return current_revisions > revisions ? ++revision : revision;
	}

	Functor functor;
	revisions_type revisions;
	revision_type revision;
	const std::tuple<supplier<Containers>...> container;
};

} // namespace internal

template<typename T, bool IsArray>
class transform_type {
private:
	typedef storage_type<T, true, IsArray> internal_storage_type;
	typedef writable_storage_type<T, IsArray> internal_writable_storage_type;
	typedef std::function<void(internal_writable_storage_type &&)> update_function_type;

	template<typename U>
	friend auto type::internal::get_revision(U &v)
		->decltype(v.get_revision())&;
	template<typename U, bool IsArray_>
	friend readable_storage_type<U, true, IsArray_> read(const transform_type<U, IsArray_> &);

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
	transform_type(internal_storage_type &&storage,
			Functor functor, const supplier<Containers> &... container)
		: impl(new internal::template_transform_type_impl<T, IsArray, Functor, Containers...>(
			std::forward<Functor>(functor), container...)),
		  storage(std::forward<internal_storage_type>(storage)) {}

	size_type size() const {
		return storage.size();
	}

private:
	void flush() const {
		impl->update(write(storage));
	}

	revision_type &get_revision() {
		return impl->get_revision();
	}

	std::unique_ptr<internal::transform_type_impl<T, IsArray>> impl;
	mutable internal_storage_type storage;
};

template<typename T>
using transform_array_type = transform_type<T, true>;
template<typename T>
using transform_primitive_type = transform_type<T, false>;

template<typename T, bool IsArray>
using read_transform_type = readable_storage_type<T, true, IsArray>;
template<typename T>
using read_transform_array_type = read_transform_type<T, true>;
template<typename T>
using read_transform_primitive_type = read_transform_type<T, false>;

template<typename T, bool IsArray>
readable_storage_type<T, true, IsArray> read(const transform_type<T, IsArray> &array) {
	array.flush();
	return read(array.storage);
}

template<typename Storage, typename... Containers, typename FunctorT>
auto make_transform(Storage &&storage, FunctorT functor, Containers... container)
		->transform_type<typename Storage::value_type, Storage::is_array> {
	return transform_type<typename Storage::value_type, Storage::is_array>(
		std::forward<Storage>(storage), std::forward<FunctorT>(functor),
		make_supplier(std::forward<Containers>(container))...);
}

}  // namespace type

#endif // TYPE_TRANSFORM_H_
