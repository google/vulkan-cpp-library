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
#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <inttypes.h>
#include <type/memory.h>
#include <type/storage.h>

namespace type {

typedef t_array<float> float_array;
typedef t_array<glm::vec2> vec2_array;
typedef t_array<glm::vec3> vec3_array;
typedef t_array<glm::vec4> vec4_array;

typedef t_array<int> int_array;
typedef t_array<glm::ivec2> ivec2_array;
typedef t_array<glm::ivec3> ivec3_array;
typedef t_array<glm::ivec4> ivec4_array;

typedef t_array<glm::mat2> mat2_array;
typedef t_array<glm::mat3> mat3_array;
typedef t_array<glm::mat4> mat4_array;

typedef t_array<uint8_t> ubyte_array;
typedef t_array<uint16_t> ushort_array;
typedef t_array<uint32_t> uint_array;

typedef t_primitive<float> float_type;
typedef t_primitive<glm::vec2> vec2;
typedef t_primitive<glm::vec3> vec3;
typedef t_primitive<glm::vec4> vec4;

typedef t_primitive<int> int_type;
typedef t_primitive<glm::ivec2> ivec2;
typedef t_primitive<glm::ivec3> ivec3;
typedef t_primitive<glm::ivec4> ivec4;

typedef t_primitive<glm::mat2> mat2;
typedef t_primitive<glm::mat3> mat3;
typedef t_primitive<glm::mat4> mat4;

typedef const_t_array<float> const_float_array;
typedef const_t_array<glm::vec2> const_vec2_array;
typedef const_t_array<glm::vec3> const_vec3_array;
typedef const_t_array<glm::vec4> const_vec4_array;

typedef const_t_array<int> const_int_array;
typedef const_t_array<glm::ivec2> const_ivec2_array;
typedef const_t_array<glm::ivec3> const_ivec3_array;
typedef const_t_array<glm::ivec4> const_ivec4_array;

typedef const_t_array<glm::mat2> const_mat2_array;
typedef const_t_array<glm::mat3> const_mat3_array;
typedef const_t_array<glm::mat4> const_mat4_array;

typedef const_t_array<uint8_t> const_ubyte_array;
typedef const_t_array<uint16_t> const_ushort_array;
typedef const_t_array<uint32_t> const_uint_array;

typedef const_t_primitive<float> const_float_type;
typedef const_t_primitive<glm::vec2> const_vec2;
typedef const_t_primitive<glm::vec3> const_vec3;
typedef const_t_primitive<glm::vec4> const_vec4;

typedef const_t_primitive<int> const_int_type;
typedef const_t_primitive<glm::ivec2> const_ivec2;
typedef const_t_primitive<glm::ivec3> const_ivec3;
typedef const_t_primitive<glm::ivec4> const_ivec4;

typedef const_t_primitive<glm::mat2> const_mat2;
typedef const_t_primitive<glm::mat3> const_mat3;
typedef const_t_primitive<glm::mat4> const_mat4;

typedef writable_t_array<float> writable_float_array;

typedef writable_t_array<glm::vec2> writable_vec2_array;
typedef writable_t_array<glm::vec3> writable_vec3_array;
typedef writable_t_array<glm::vec4> writable_vec4_array;

typedef writable_t_array<int> writable_int_array;
typedef writable_t_array<glm::ivec2> writable_ivec2_array;
typedef writable_t_array<glm::ivec3> writable_ivec3_array;
typedef writable_t_array<glm::ivec4> writable_ivec4_array;

typedef writable_t_array<glm::mat2> writable_mat2_array;
typedef writable_t_array<glm::mat3> writable_mat3_array;
typedef writable_t_array<glm::mat4> writable_mat4_array;

typedef writable_t_array<uint8_t> writable_ubyte_array;
typedef writable_t_array<uint16_t> writable_ushort_array;

typedef writable_t_primitive<float> writable_float;

typedef writable_t_primitive<glm::vec2> writable_vec2;
typedef writable_t_primitive<glm::vec3> writable_vec3;
typedef writable_t_primitive<glm::vec4> writable_vec4;

typedef writable_t_primitive<int> writable_int;
typedef writable_t_primitive<glm::ivec2> writable_ivec2;
typedef writable_t_primitive<glm::ivec3> writable_ivec3;
typedef writable_t_primitive<glm::ivec4> writable_ivec4;

typedef writable_t_primitive<glm::mat2> writable_mat2;
typedef writable_t_primitive<glm::mat3> writable_mat3;
typedef writable_t_primitive<glm::mat4> writable_mat4;

typedef writable_t_primitive<uint8_t> writable_ubyte;
typedef writable_t_primitive<uint16_t> writable_ushort;

typedef readable_t_array<float, true> readable_float_array;

typedef readable_t_array<glm::vec2, true> readable_vec2_array;
typedef readable_t_array<glm::vec3, true> readable_vec3_array;
typedef readable_t_array<glm::vec4, true> readable_vec4_array;

typedef readable_t_array<int, true> readable_int_array;
typedef readable_t_array<glm::ivec2, true> readable_ivec2_array;
typedef readable_t_array<glm::ivec3, true> readable_ivec3_array;
typedef readable_t_array<glm::ivec4, true> readable_ivec4_array;

typedef readable_t_array<glm::mat2, true> readable_mat2_array;
typedef readable_t_array<glm::mat3, true> readable_mat3_array;
typedef readable_t_array<glm::mat4, true> readable_mat4_array;

typedef readable_t_array<uint8_t, true> readable_ubyte_array;
typedef readable_t_array<uint16_t, true> readable_ushort_array;

typedef readable_t_primitive<float, true> readable_float;

typedef readable_t_primitive<glm::vec2, true> readable_vec2;
typedef readable_t_primitive<glm::vec3, true> readable_vec3;
typedef readable_t_primitive<glm::vec4, true> readable_vec4;

typedef readable_t_primitive<int, true> readable_int;
typedef readable_t_primitive<glm::ivec2, true> readable_ivec2;
typedef readable_t_primitive<glm::ivec3, true> readable_ivec3;
typedef readable_t_primitive<glm::ivec4, true> readable_ivec4;

typedef readable_t_primitive<glm::mat2, true> readable_mat2;
typedef readable_t_primitive<glm::mat3, true> readable_mat3;
typedef readable_t_primitive<glm::mat4, true> readable_mat4;

typedef readable_t_primitive<uint8_t, true> readable_ubyte;
typedef readable_t_primitive<uint16_t, true> readable_ushort;

typedef readable_t_array<float, false> readable_const_float_array;

typedef readable_t_array<glm::vec2, false> readable_const_vec2_array;
typedef readable_t_array<glm::vec3, false> readable_const_vec3_array;
typedef readable_t_array<glm::vec4, false> readable_const_vec4_array;

typedef readable_t_array<int, false> readable_const_int_array;
typedef readable_t_array<glm::ivec2, false> readable_const_ivec2_array;
typedef readable_t_array<glm::ivec3, false> readable_const_ivec3_array;
typedef readable_t_array<glm::ivec4, false> readable_const_ivec4_array;

typedef readable_t_array<glm::mat2, false> readable_const_mat2_array;
typedef readable_t_array<glm::mat3, false> readable_const_mat3_array;
typedef readable_t_array<glm::mat4, false> readable_const_mat4_array;

typedef readable_t_array<uint8_t, false> readable_const_ubyte_array;
typedef readable_t_array<uint16_t, false> readable_const_ushort_array;

typedef readable_t_primitive<float, false> readable_const_float;

typedef readable_t_primitive<glm::vec2, false> readable_const_vec2;
typedef readable_t_primitive<glm::vec3, false> readable_const_vec3;
typedef readable_t_primitive<glm::vec4, false> readable_const_vec4;

typedef readable_t_primitive<int, false> readable_const_int;
typedef readable_t_primitive<glm::ivec2, false> readable_const_ivec2;
typedef readable_t_primitive<glm::ivec3, false> readable_const_ivec3;
typedef readable_t_primitive<glm::ivec4, false> readable_const_ivec4;

typedef readable_t_primitive<glm::mat2, false> readable_const_mat2;
typedef readable_t_primitive<glm::mat3, false> readable_const_mat3;
typedef readable_t_primitive<glm::mat4, false> readable_const_mat4;

typedef readable_t_primitive<uint8_t, false> readable_const_ubyte;
typedef readable_t_primitive<uint16_t, false> readable_const_ushort;

enum element_enum {
	type_unknown = 0,
	type_float,
	type_int,
	type_uint,
	type_ubyte,
	type_ushort
};

namespace internal {

template<memory_layout layout>
struct alignment_type {

	constexpr static std::size_t align_offset(std::size_t base_offset, std::size_t alignment) {
		return base_offset + (alignment - base_offset % alignment) % alignment;
	}

	constexpr static std::size_t array_alignment(std::size_t alignment) {
		return layout == linear_std140 || layout == interleaved_std140
			? (std::max)(alignment, sizeof(float) * 4) : alignment;
	}

	template<typename T, bool IsArray>
	constexpr static std::size_t size() {
		typedef primitive_type_information<layout, T> type_info;
		return layout != interleaved_std140 && layout != interleaved_std430 && IsArray
			? array_alignment(type_info::array_size) : type_info::size;
	}

	template<typename T, bool IsArray>
	constexpr static std::size_t alignment() {
		typedef primitive_type_information<layout, T> type_info;
		return layout == linear_std140 && IsArray
			? (std::max)(type_info::alignment, sizeof(float) * 4) : type_info::alignment;
	}

	constexpr static std::size_t struct_alignment(std::size_t max_alignment) {
		return array_alignment(max_alignment);
	}
};

template<>
struct alignment_type<linear> {

	constexpr static std::size_t align_offset(std::size_t offset, std::size_t) {
		return offset;
	}

	template<typename T, bool IsArray>
	constexpr static std::size_t size() {
		return primitive_type_information<linear, T>::size;
	}

	template<typename T, bool IsArray>
	constexpr static std::size_t alignment() {
		return 1;
	}

	constexpr static std::size_t struct_alignment(std::size_t) {
		return 1;
	}
};

template<element_enum Element, std::size_t Cols, std::size_t Rows, bool IsArray>
struct type_information_traits {
	constexpr static element_enum element = Element;
	constexpr static std::size_t cols = Cols, rows = Rows;
	constexpr static bool is_array = IsArray;
};

template<memory_layout layout, typename T>
struct primitive_type_information;

template<typename T> struct primitive_primitive_type_information {

	constexpr static std::size_t size = sizeof(T), alignment = sizeof(T), array_size = size;

	static void copy(const T &value, void *target) {
		*reinterpret_cast<T *>(target) = value;
	}
};
template<typename T>
constexpr std::size_t primitive_primitive_type_information<T>::size;
template<typename T>
constexpr std::size_t primitive_primitive_type_information<T>::alignment;

template<memory_layout layout> struct primitive_type_information<layout, float>
	: primitive_primitive_type_information<float> {};
template<memory_layout layout> struct primitive_type_information<layout, double>
	: primitive_primitive_type_information<double> {};
template<memory_layout layout> struct primitive_type_information<layout, int8_t>
	: primitive_primitive_type_information<int8_t> {};
template<memory_layout layout> struct primitive_type_information<layout, uint8_t>
	: primitive_primitive_type_information<uint8_t> {};
template<memory_layout layout> struct primitive_type_information<layout, int16_t>
	: primitive_primitive_type_information<int16_t> {};
template<memory_layout layout> struct primitive_type_information<layout, uint16_t>
	: primitive_primitive_type_information<uint16_t> {};
template<memory_layout layout> struct primitive_type_information<layout, int32_t>
	: primitive_primitive_type_information<int32_t> {};
template<memory_layout layout> struct primitive_type_information<layout, uint32_t>
	: primitive_primitive_type_information<uint32_t> {};

template<typename T, std::size_t Size, std::size_t Alignment = Size>
struct glm_vec_type_information {

	constexpr static std::size_t size = Size, alignment = Alignment, array_size = alignment;

	static void copy(const T &value, void *target) {
		std::memcpy(target, glm::value_ptr(value), size);
	}
};
template<typename T, std::size_t Size, std::size_t Alignment>
constexpr std::size_t glm_vec_type_information<T, Size, Alignment>::size;
template<typename T, std::size_t Size, std::size_t Alignment>
constexpr std::size_t glm_vec_type_information<T, Size, Alignment>::alignment;

template<memory_layout layout> struct primitive_type_information<layout, glm::vec2>
	: glm_vec_type_information<glm::vec2, sizeof(float) * 2> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::vec3>
	: glm_vec_type_information<glm::vec3, sizeof(float) * 3, sizeof(float) * 4> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::vec4>
	: glm_vec_type_information<glm::vec4, sizeof(float) * 4> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::ivec2>
	: glm_vec_type_information<glm::ivec2, sizeof(int32_t) * 2> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::ivec3>
	: glm_vec_type_information<glm::ivec3, sizeof(int32_t) * 3, sizeof(float) * 4> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::ivec4>
	: glm_vec_type_information<glm::ivec4, sizeof(int32_t) * 4> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::uvec2>
	: glm_vec_type_information<glm::uvec2, sizeof(uint32_t) * 2> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::uvec3>
	: glm_vec_type_information<glm::uvec3, sizeof(uint32_t) * 3, sizeof(float) * 4> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::uvec4>
	: glm_vec_type_information<glm::uvec4, sizeof(uint32_t) * 4> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::bvec2>
	: glm_vec_type_information<glm::bvec2, sizeof(bool) * 2> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::bvec3>
	: glm_vec_type_information<glm::bvec3, sizeof(bool) * 3, sizeof(bool) * 4> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::bvec4>
	: glm_vec_type_information<glm::bvec4, sizeof(bool) * 4> {};

template<memory_layout layout, typename T, std::size_t Size>
struct primitive_type_information<layout, std::array<T, Size>> {
	constexpr static std::size_t primitive_alignment = primitive_type_information<layout, T>::alignment;
	constexpr static std::size_t alignment = layout == linear_std140 || layout == interleaved_std140
		? (std::max)(primitive_alignment, sizeof(float) * 4) : primitive_alignment,
		size = Size * alignment, array_size = size;

	static void copy(const std::array<T, Size> &value, void *target) {
		uint8_t *bytes(reinterpret_cast<uint8_t *>(target));
		for (const T &element : value) {
			primitive_type_information<layout, T>::copy(element, bytes);
			bytes += alignment;
		}
	}
};

template<typename T>
struct is_array_type {
	constexpr static bool value = false;
};
template<typename T, std::size_t Size>
struct is_array_type<std::array<T, Size>> {
	constexpr static bool value = true;
};

template<memory_layout layout, std::size_t I>
struct tuple_type_information_size_type {

	template<typename Tuple>
	constexpr static std::size_t offset() {
		typedef typename std::tuple_element<I - 1, Tuple>::type type;
		typedef primitive_type_information<layout, type> type_info;
		return alignment_type<layout>::align_offset(
			tuple_type_information_size_type<layout, I - 1>::size<Tuple>(), type_info::alignment);

	}

	template<typename Tuple>
	constexpr static std::size_t size() {
		typedef typename std::tuple_element<I - 1, Tuple>::type type;
		typedef primitive_type_information<layout, type> type_info;
		return offset<Tuple>()
			+ alignment_type<layout>::template size<type, is_array_type<type>::value>();
	}

	template<typename Tuple>
	constexpr static std::size_t alignment() {
		typedef typename std::tuple_element<I - 1, Tuple>::type type;
		return (std::max)(tuple_type_information_size_type<layout, I - 1>::alignment<Tuple>(),
			alignment_type<layout>::template alignment<type, is_array_type<type>::value>());
	}

	template<typename Tuple>
	static void copy(const Tuple &value, void *target) {
		typedef std::tuple_element<I - 1, Tuple>::type type;
		typedef primitive_type_information<layout, type> type_info;
		tuple_type_information_size_type<layout, I - 1>::copy(value, target);
		type_info::copy(std::get<I - 1>(value), reinterpret_cast<uint8_t *>(target)
			+ offset<Tuple>());
	}
};

template<memory_layout layout>
struct tuple_type_information_size_type<layout, 0> {

	template<typename Tuple>
	constexpr static std::size_t size() {
		return 0;
	}

	template<typename Tuple>
	constexpr static std::size_t alignment() {
		return layout == linear_std140 || layout == interleaved_std140 ? sizeof(float) * 4 : 0;
	}

	template<typename Tuple>
	static std::size_t copy(const Tuple &value, void *target) {
		return 0;
	}
};

template<memory_layout Layout, typename... Ts>
struct primitive_type_information<Layout, std::tuple<Ts...>> {
	typedef tuple_type_information_size_type<Layout, sizeof...(Ts)> type_info;
	constexpr static std::size_t size = type_info::template size<std::tuple<Ts...>>();
	constexpr static std::size_t alignment = type_info::template alignment<std::tuple<Ts...>>();
	constexpr static std::size_t array_size = alignment_type<Layout>::align_offset(size,
		alignment_type<Layout>::array_alignment(alignment));

	static void copy(const std::tuple<Ts...> &value, void *target) {
		type_info::copy(value, target);
	}
};

template<typename T, std::size_t Size, std::size_t Alignment, std::size_t Columns>
struct glm_mat_type_information {

	constexpr static std::size_t size = Columns * Alignment, alignment = Alignment, array_size = size;

	static void copy(const T &value, void *target) {
		uint8_t *bytes(reinterpret_cast<uint8_t *>(target));
		for (std::size_t column = 0; column < Columns; ++column) {
			std::memcpy(bytes, glm::value_ptr(value[column]), Size);
			bytes += alignment;
		}
	}
};

template<typename T, std::size_t Size, std::size_t Alignment, std::size_t Columns>
constexpr std::size_t glm_mat_type_information<T, Size, Alignment, Columns>::size;
template<typename T, std::size_t Size, std::size_t Alignment, std::size_t Columns>
constexpr std::size_t glm_mat_type_information<T, Size, Alignment, Columns>::alignment;

template<memory_layout layout> struct primitive_type_information<layout, glm::mat2>
	: glm_mat_type_information<glm::mat2, sizeof(float) * 2, sizeof(float) * 2, 2> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::mat2x3>
	: glm_mat_type_information<glm::mat2x3, sizeof(float) * 3, sizeof(float) * 4, 2> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::mat2x4>
	: glm_mat_type_information<glm::mat2x4, sizeof(float) * 4, sizeof(float) * 4, 2> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::mat3x2>
	: glm_mat_type_information<glm::mat3x2, sizeof(float) * 2, sizeof(float) * 2, 3> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::mat3>
	: glm_mat_type_information<glm::mat3, sizeof(float) * 3, sizeof(float) * 4, 3> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::mat3x4>
	: glm_mat_type_information<glm::mat3x4, sizeof(float) * 4, sizeof(float) * 4, 3> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::mat4>
	: glm_mat_type_information<glm::mat4, sizeof(float) * 4, sizeof(float) * 4, 4> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::mat4x2>
	: glm_mat_type_information<glm::mat4x2, sizeof(float) * 2, sizeof(float) * 2, 4> {};
template<memory_layout layout> struct primitive_type_information<layout, glm::mat4x3>
	: glm_mat_type_information<glm::mat4x3, sizeof(float) * 3, sizeof(float) * 4, 4> {};

}  // namespace internal

template<typename T>
struct type_information;

template<>
struct type_information<float_array>
	: internal::type_information_traits<type_float, 1, 1, true> {};
template<>
struct type_information<vec2_array> : internal::type_information_traits<type_float, 2, 1, true> {};
template<>
struct type_information<vec3_array> : internal::type_information_traits<type_float, 3, 1, true> {};
template<>
struct type_information<vec4_array> : internal::type_information_traits<type_float, 4, 1, true> {};
template<>
struct type_information<int_array> : internal::type_information_traits<type_int, 1, 1, true> {};
template<>
struct type_information<ivec2_array> : internal::type_information_traits<type_int, 2, 1, true> {};
template<>
struct type_information<ivec3_array> : internal::type_information_traits<type_int, 3, 1, true> {};
template<>
struct type_information<ivec4_array> : internal::type_information_traits<type_int, 4, 1, true> {};
template<>
struct type_information<mat2_array> : internal::type_information_traits<type_float, 2, 2, true> {};
template<>
struct type_information<mat3_array> : internal::type_information_traits<type_float, 3, 3, true> {};
template<>
struct type_information<mat4_array> : internal::type_information_traits<type_float, 4, 4, true> {};
template<>
struct type_information<ubyte_array>
	: internal::type_information_traits<type_ubyte, 1, 1, true> {};
template<>
struct type_information<ushort_array>
	: internal::type_information_traits<type_ushort, 1, 1, true> {};
template<>
struct type_information<uint_array> : internal::type_information_traits<type_uint, 1, 1, true> {};

template<>
struct type_information<float_type>
	: internal::type_information_traits<type_float, 1, 1, false> {};
template<>
struct type_information<vec2> : internal::type_information_traits<type_float, 2, 1, false> {};
template<>
struct type_information<vec3> : internal::type_information_traits<type_float, 3, 1, false> {};
template<>
struct type_information<vec4> : internal::type_information_traits<type_float, 4, 1, false> {};
template<>
struct type_information<int> : internal::type_information_traits<type_int, 1, 1, false> {};
template<>
struct type_information<ivec2> : internal::type_information_traits<type_int, 2, 1, false> {};
template<>
struct type_information<ivec3> : internal::type_information_traits<type_int, 3, 1, false> {};
template<>
struct type_information<ivec4> : internal::type_information_traits<type_int, 4, 1, false> {};
template<>
struct type_information<mat2> : internal::type_information_traits<type_float, 2, 2, false> {};
template<>
struct type_information<mat3> : internal::type_information_traits<type_float, 3, 3, false> {};
template<>
struct type_information<mat4> : internal::type_information_traits<type_float, 4, 4, false> {};

template<>
struct type_information<const_float_array> : type_information<float_array> {};
template<>
struct type_information<const_vec2_array> : type_information<vec2_array> {};
template<>
struct type_information<const_vec3_array> : type_information<vec3_array> {};
template<>
struct type_information<const_vec4_array> : type_information<vec4_array> {};
template<>
struct type_information<const_int_array> : type_information<int_array> {};
template<>
struct type_information<const_ivec2_array> : type_information<ivec2_array> {};
template<>
struct type_information<const_ivec3_array> : type_information<ivec3_array> {};
template<>
struct type_information<const_ivec4_array> : type_information<ivec4_array> {};
template<>
struct type_information<const_mat2_array> : type_information<mat2_array> {};
template<>
struct type_information<const_mat3_array> : type_information<mat3_array> {};
template<>
struct type_information<const_mat4_array> : type_information<mat4_array> {};
template<>
struct type_information<const_ubyte_array> : type_information<ubyte_array> {};
template<>
struct type_information<const_ushort_array> : type_information<ushort_array> {};
template<>
struct type_information<const_float_type> : type_information<float_type> {};
template<>
struct type_information<const_vec2> : type_information<vec2> {};
template<>
struct type_information<const_vec3> : type_information<vec3> {};
template<>
struct type_information<const_vec4> : type_information<vec4> {};
template<>
struct type_information<const_ivec2> : type_information<ivec2> {};
template<>
struct type_information<const_ivec3> : type_information<ivec3> {};
template<>
struct type_information<const_ivec4> : type_information<ivec4> {};
template<>
struct type_information<const_mat2> : type_information<mat2> {};
template<>
struct type_information<const_mat3> : type_information<mat3> {};
template<>
struct type_information<const_mat4> : type_information<mat4> {};
template<>
struct type_information<writable_float_array> : type_information<float_array> {};
template<>
struct type_information<writable_vec2_array> : type_information<vec2_array> {};
template<>
struct type_information<writable_vec3_array> : type_information<vec3_array> {};
template<>
struct type_information<writable_vec4_array> : type_information<vec4_array> {};
template<>
struct type_information<writable_int_array> : type_information<int_array> {};
template<>
struct type_information<writable_ivec2_array> : type_information<ivec2_array> {};
template<>
struct type_information<writable_ivec3_array> : type_information<ivec3_array> {};
template<>
struct type_information<writable_ivec4_array> : type_information<ivec4_array> {};
template<>
struct type_information<writable_mat2_array> : type_information<mat2_array> {};
template<>
struct type_information<writable_mat3_array> : type_information<mat3_array> {};
template<>
struct type_information<writable_mat4_array> : type_information<mat4_array> {};
template<>
struct type_information<writable_ubyte_array> : type_information<ubyte_array> {};
template<>
struct type_information<writable_ushort_array> : type_information<ushort_array> {};

}  // namespace type

#endif // DATA_TYPES_H_
