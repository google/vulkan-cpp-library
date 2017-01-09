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

template<std::size_t I>
struct layout_type {
	typedef std::array<std::size_t, I> indices_type;
	indices_type offset, stride;
	std::size_t size;
};

template<memory_layout layout>
struct alignment_type {

	constexpr static std::size_t align_offset(std::size_t base_offset, std::size_t alignment) {
		return base_offset + (alignment - base_offset % alignment) % alignment;
	}

	template<typename T>
	static std::size_t size() {
		auto primitive_alignment(primitive_type_information<typename T::value_type>::alignment);
		auto std140(layout == linear_std140 || layout == interleaved_std140);
		auto interleaved(layout == interleaved_std140 || layout == interleaved_std430);
		auto alignment(std140 ? (std::max)(primitive_alignment, sizeof(float) * 4)
			: primitive_alignment);
		auto columns(primitive_type_information<typename T::value_type>::columns);
		auto is_array(columns != 1 || (!interleaved && T::is_array));
		return is_array ? columns * alignment
			: primitive_type_information<typename T::value_type>::size;
	}

	template<typename T>
	static std::size_t alignment() {
		typedef primitive_type_information<typename T::value_type> type_info;
		return layout == linear_std140 && T::is_array
			? (std::max)(type_info::alignment, sizeof(float) * 4) : type_info::alignment;
	}

	static std::size_t struct_alignment(std::size_t max_alignment) {
		auto std140(layout == linear_std140 || layout == interleaved_std140);
		return std140 ? (std::max)(max_alignment, sizeof(float) * 4) : max_alignment;
	}
};

template<>
struct alignment_type<linear> {

	constexpr static std::size_t align_offset(std::size_t base_offset, std::size_t alignment) {
		return base_offset;
	}

	template<typename T>
	constexpr static std::size_t size() {
		return primitive_type_information<typename T::value_type>::size;
	}

	template<typename T>
	static std::size_t alignment() {
		return 1;
	}

	static std::size_t struct_alignment(std::size_t max_alignment) {
		return 1;
	}
};

template<memory_layout layout>
struct calculate_linear_layout_type {
	template<typename... Storage>
	static layout_type<sizeof...(Storage)> calculate(const Storage &... storage) {
		typedef alignment_type<layout> alignment_type;
		typedef std::array<std::size_t, sizeof...(Storage)> indices_type;

		const indices_type alignment{ alignment_type::alignment<Storage>()... },
			elements{ storage.size()... }, size{ alignment_type::size<Storage>()... };

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
	static layout_type<sizeof...(Storage)> calculate(const Storage &... storage) {
		typedef alignment_type<layout> alignment_type;
		typedef std::array<std::size_t, sizeof...(Storage)> indices_type;

		const indices_type alignment{ alignment_type::alignment<Storage>()... },
			elements{ storage.size()... }, size{ alignment_type::size<Storage>()... };

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

template<typename... Storage>
struct serialize_type {

	typedef layout_type<sizeof...(Storage)> layout_type;

	template<memory_layout layout>
	serialize_type(const supplier<Storage>&... storages)
		: layout(calculate_layout_type<layout, Storage...>::calculate(*storages...))
		, storages(storages...) {}

	void operator()(void *target) const {

	}

	const layout_type layout;
	const std::tuple<supplier<Storage>&...> storages;
};

template<typename Storage>
void serialize(const Storage &storage, std::size_t offset, std::size_t stride, void *target) {
	uint8_t *bytes(reinterpret_cast<uint8_t *>(target) + offset);

	auto read(read(storage));
	for (const auto &value : read) {
		uint8_t *row_bytes(bytes);
		for (std::size_t i = 0;
				i < primitive_type_information<typename Storage::value_type>::columns; ++i) {
			std::memcpy(row_bytes, primitive_type_information<typename Storage::value_type>::ptr(value, i),
				primitive_type_information<typename Storage::value_type>::size);
			row_bytes += primitive_type_information<typename Storage::value_type>::alignment;
		}
		bytes += stride;
	}
}

template<std::size_t I>
struct serialize_storage_type {

	template<typename Layout, typename Storages>
	static void serialize(const Layout &layout, const Storages &storages, void *target) {
		constexpr std::size_t index = I - 1;
		internal::serialize(*std::get<index>(storages), std::get<index>(layout.offset),
			std::get<index>(layout.stride), target);
		serialize_storage_type<index>::serialize(layout, storages, target);
	}
};

template<>
struct serialize_storage_type<0> {
	template<typename Layout, typename Storages>
	static void serialize(const Layout &layout, const Storages &storages, void *target) {}
};

template<typename... Storage>
void serialize_storages(const layout_type<sizeof...(Storage)> &layout,
		const supplier<Storage>&... storages, void *target) {
	serialize_storage_type<sizeof...(Storage)>::serialize(layout, std::tie(storages...), target);
}

} // namespace internal

struct serialize_type {

	typedef std::function<void(void *)> function_type;

	serialize_type() = default;

	template<typename Layout, typename... Storage>
	explicit serialize_type(Layout &&layout, const supplier<Storage>&... storages)
		: function(std::bind(&internal::serialize_storages<Storage...>,
			std::forward<Layout>(layout), storages..., std::placeholders::_1))
		, size(layout.size) {}

	function_type function;
	std::size_t size;
};

template<memory_layout Layout, typename... Storages>
serialize_type make_serialize(const type::supplier<Storages> &... storages) {
	return serialize_type(internal::calculate_layout_type<Layout>::calculate(*storages...),
		storages...);
}

inline void flush(const serialize_type &serialize, void *target) {
	serialize.function(target);
}

inline std::size_t size(const serialize_type &serialize) {
	return serialize.size;
}

inline bool dirty(const serialize_type &serialize) {
	// TODO(gardell): serialize must store revisions
	return true;
}

}  // namespace type

#endif // TYPE_SERIALIZE_H_
