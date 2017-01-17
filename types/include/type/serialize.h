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
#ifndef TYPE_SERIALIZE_H_
#define TYPE_SERIALIZE_H_

#include <algorithm>
#include <array>
#include <type/memory.h>
#include <type/supplier.h>
#include <type/types.h>

namespace type {
namespace internal {

template<memory_layout Layout, std::size_t I>
struct layout_type {
	constexpr static memory_layout layout = Layout;
	typedef std::array<std::size_t, I> indices_type;
	indices_type offset, stride;
	std::size_t size;
};

template<memory_layout layout>
struct calculate_linear_layout_type {
	template<typename... Storage>
	static layout_type<layout, sizeof...(Storage)> calculate(const Storage &... storage) {
		typedef alignment_type<layout> alignment_type;
		typedef std::array<std::size_t, sizeof...(Storage)> indices_type;

		const indices_type alignment{ alignment_type
				::template alignment<typename Storage::value_type, Storage::is_array>()... },
			elements{ storage.size()... },
			size{ alignment_type
				::template size<typename Storage::value_type, Storage::is_array>()... };

		indices_type offset;
		offset.front() = 0;
		for (std::size_t i = 1; i < sizeof...(Storage); ++i) {
			offset[i] = alignment_type::align_offset(offset[i - 1] + elements[i - 1] * size[i - 1],
				alignment[i]);
		}
		return{ offset, size, offset.back() + elements.back() * size.back() };
	}
};

template<memory_layout layout>
struct calculate_interleaved_layout_type {

	template<typename... Storage>
	static layout_type<layout, sizeof...(Storage)> calculate(const Storage &... storage) {
		typedef alignment_type<layout> alignment_type;
		typedef std::array<std::size_t, sizeof...(Storage)> indices_type;

		const indices_type alignment{ alignment_type
				::template alignment<typename Storage::value_type, Storage::is_array>()... },
			elements{ storage.size()... },
			size{ alignment_type
				::template size<typename Storage::value_type, Storage::is_array>()... };

		indices_type offsets, strides;
		std::size_t start = 0;
		std::size_t offset = 0;
		for (std::size_t i = 1; i <= sizeof...(Storage); ++i) {
			if (i == sizeof...(Storage) || elements[i] != elements[start]) {
				const std::size_t end = i;
				const std::size_t struct_alignment(alignment_type::struct_alignment(*std::max_element(
					std::begin(alignment) + start, std::begin(alignment) + end)));
				offsets[start] = alignment_type::align_offset(offset, struct_alignment);
				for (std::size_t j = start + 1; j < end; ++j) {
					offsets[j] = alignment_type::align_offset(offsets[j - 1] + size[j - 1],
						alignment[j]);
				}
				std::size_t stride = alignment_type::align_offset(offsets[end - 1]
					+ size.back() - offsets[start], struct_alignment);
				std::fill(std::begin(strides) + start, std::begin(strides) + end,
					stride);
				offset = offsets[start] + stride * elements[start];
				start = end;
			}
		}
		return{ offsets, strides, offset };
	}
};

template<memory_layout, typename... Storage>
struct calculate_layout_type;

template<> struct calculate_layout_type<linear>
	: calculate_linear_layout_type<linear> {};
template<> struct calculate_layout_type<linear_std140>
	: calculate_linear_layout_type<linear_std140> {};
template<> struct calculate_layout_type<linear_std430>
	: calculate_linear_layout_type<linear_std430> {};

template<> struct calculate_layout_type<interleaved_std140>
	: calculate_interleaved_layout_type<interleaved_std140> {};

template<> struct calculate_layout_type<interleaved_std430>
	: calculate_interleaved_layout_type<interleaved_std430> {};

template<memory_layout Layout, typename Storage>
void serialize(const Storage &storage, std::size_t offset, std::size_t stride, void *target) {
	uint8_t *bytes(reinterpret_cast<uint8_t *>(target) + offset);

	auto values(read(storage));
	for (const auto &value : values) {
		primitive_type_information<Layout, typename Storage::value_type>::copy(value, bytes);
		bytes += stride;
	}
}

template<std::size_t I>
struct serialize_storage_type {

	template<typename Layout, typename Storages>
	static void serialize(const Layout &layout, const Storages &storages, void *target) {
		constexpr std::size_t index = I - 1;
		internal::serialize<Layout::layout>(*std::get<index>(storages),
			std::get<index>(layout.offset), std::get<index>(layout.stride), target);
		serialize_storage_type<index>::serialize(layout, storages, target);
	}
};

template<>
struct serialize_storage_type<0> {
	template<typename Layout, typename Storages>
	static void serialize(const Layout &layout, const Storages &storages, void *target) {}
};

template<std::size_t I>
struct serialize_revision_type {

	template<typename Layout, typename Storages>
	static std::array<revision_type, std::tuple_size<Storages>::value> revision(
			const Layout &layout, const Storages &storages) {
		auto revision(serialize_revision_type<I - 1>::revision(layout, storages));
		std::get<I - 1>(revision) = get_revision(*std::get<I - 1>(storages));
		return revision;
	}
};

template<>
struct serialize_revision_type<0> {

	template<typename Layout, typename Storages>
	static std::array<revision_type, std::tuple_size<Storages>::value> revision(
			const Layout &layout, const Storages &storages) {
		return std::array<revision_type, std::tuple_size<Storages>::value>();
	}
};

struct serialize_type_impl {

	virtual void flush(void *target) = 0;
	virtual bool dirty() const = 0;
};

template<typename Layout, typename Storages>
struct template_serialize_type_impl : serialize_type_impl {

	template_serialize_type_impl(Layout &&layout, const Storages &storages)
		: layout(std::forward<Layout>(layout)), storages(storages) {
		std::fill(std::begin(revision), std::end(revision), REVISION_NONE);
	}

	virtual void flush(void *target) override {
		serialize_storage_type<std::tuple_size<Storages>::value>
			::serialize(layout, storages, target);
		revision = serialize_revision_type<std::tuple_size<Storages>::value>::revision(layout,
			storages);
	}

	virtual bool dirty() const override {
		auto current_revision(serialize_revision_type<std::tuple_size<Storages>::value>::revision(
			layout, storages));
		return current_revision > revision;
	}

	Layout layout;
	Storages storages;
	std::array<revision_type, std::tuple_size<Storages>::value> revision;
};

template<typename Layout, typename... Storage>
std::unique_ptr<serialize_type_impl> create_serialize_impl(Layout &&layout,
		const supplier<Storage>&... storages) {
	typedef std::tuple<const supplier<Storage>...> storage_type;
	return std::unique_ptr<serialize_type_impl>(
		new internal::template_serialize_type_impl<Layout, storage_type>(
			std::forward<Layout>(layout), std::make_tuple(storages...)));
}

} // namespace internal

struct serialize_type {

	typedef std::function<void(void *)> function_type;

	serialize_type() = default;

	template<typename Layout, typename... Storage>
	explicit serialize_type(Layout &&layout, const supplier<Storage>&... storages)
		: size(layout.size)
		, impl(internal::create_serialize_impl(std::forward<Layout>(layout), storages...)) {}

	std::unique_ptr<internal::serialize_type_impl> impl;
	std::size_t size;
};

template<memory_layout Layout, typename... Storages>
serialize_type make_serialize(const type::supplier<Storages> &... storages) {
	return serialize_type(internal::calculate_layout_type<Layout>::calculate(*storages...),
		storages...);
}

inline void flush(const serialize_type &serialize, void *target) {
	serialize.impl->flush(target);
}

inline std::size_t size(const serialize_type &serialize) {
	return serialize.size;
}

inline bool dirty(const serialize_type &serialize) {
	return serialize.impl->dirty();
}

}  // namespace type

#endif // TYPE_SERIALIZE_H_
