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

	template<element_enum Element, std::size_t Cols, std::size_t Rows, bool IsArray>
	struct type_information_traits {
		constexpr static element_enum element = Element;
		constexpr static std::size_t cols = Cols, rows = Rows;
		constexpr static bool is_array = IsArray;
	};

	template<typename T>
	struct primitive_type_information;

	template<typename T> struct primitive_primitive_type_information {

		constexpr static std::size_t size = sizeof(T), alignment = sizeof(T);

		static void copy(const T &value, void *target) {
			*reinterpret_cast<T *>(target) = value;
		}
	};
	template<typename T> constexpr std::size_t primitive_primitive_type_information<T>::size;
	template<typename T> constexpr std::size_t primitive_primitive_type_information<T>::alignment;

	template<> struct primitive_type_information<float>
		: primitive_primitive_type_information<float> {};
	template<> struct primitive_type_information<double>
		: primitive_primitive_type_information<double> {};
	template<> struct primitive_type_information<int8_t>
		: primitive_primitive_type_information<int8_t> {};
	template<> struct primitive_type_information<uint8_t>
		: primitive_primitive_type_information<uint8_t> {};
	template<> struct primitive_type_information<int16_t>
		: primitive_primitive_type_information<int16_t> {};
	template<> struct primitive_type_information<uint16_t>
		: primitive_primitive_type_information<uint16_t> {};
	template<> struct primitive_type_information<int32_t>
		: primitive_primitive_type_information<int32_t> {};
	template<> struct primitive_type_information<uint32_t>
		: primitive_primitive_type_information<uint32_t> {};

	template<typename T, std::size_t Size, std::size_t Alignment = Size>
	struct glm_vec_type_information {

		constexpr static std::size_t size = Size, alignment = Alignment;

		static void copy(const T &value, void *target) {
			std::memcpy(target, glm::value_ptr(value), size);
		}
	};
	template<typename T, std::size_t Size, std::size_t Alignment>
	constexpr std::size_t glm_vec_type_information<T, Size, Alignment>::size;
	template<typename T, std::size_t Size, std::size_t Alignment>
	constexpr std::size_t glm_vec_type_information<T, Size, Alignment>::alignment;

	template<> struct primitive_type_information<glm::vec2>
		: glm_vec_type_information<glm::vec2, sizeof(float) * 2> {};
	template<> struct primitive_type_information<glm::vec3>
		: glm_vec_type_information<glm::vec3, sizeof(float) * 3, sizeof(float) * 4> {};
	template<> struct primitive_type_information<glm::vec4>
		: glm_vec_type_information<glm::vec4, sizeof(float) * 4> {};
	template<> struct primitive_type_information<glm::ivec2>
		: glm_vec_type_information<glm::ivec2, sizeof(int32_t) * 2> {};
	template<> struct primitive_type_information<glm::ivec3>
		: glm_vec_type_information<glm::ivec3, sizeof(int32_t) * 3, sizeof(float) * 4> {};
	template<> struct primitive_type_information<glm::ivec4>
		: glm_vec_type_information<glm::ivec4, sizeof(int32_t) * 4> {};
	template<> struct primitive_type_information<glm::uvec2>
		: glm_vec_type_information<glm::uvec2, sizeof(uint32_t) * 2> {};
	template<> struct primitive_type_information<glm::uvec3>
		: glm_vec_type_information<glm::uvec3, sizeof(uint32_t) * 3, sizeof(float) * 4> {};
	template<> struct primitive_type_information<glm::uvec4>
		: glm_vec_type_information<glm::uvec4, sizeof(uint32_t) * 4> {};
	template<> struct primitive_type_information<glm::bvec2>
		: glm_vec_type_information<glm::bvec2, sizeof(bool) * 2> {};
	template<> struct primitive_type_information<glm::bvec3>
		: glm_vec_type_information<glm::bvec3, sizeof(bool) * 3, sizeof(bool) * 4> {};
	template<> struct primitive_type_information<glm::bvec4>
		: glm_vec_type_information<glm::bvec4, sizeof(bool) * 4> {};

	// TODO(gardell): Write test!
	// TODO(gardell): Alignment is dependent on std140/std430!
	template<typename T, std::size_t Size>
	struct primitive_type_information<std::array<T, Size>> {
		constexpr static std::size_t alignment = primitive_type_information<T>::alignment,
			element_size = primitive_type_information<T>::size,
			size = Size * alignment;

		static void copy(const T &value, void *target) {
			uint8_t *bytes(reinterpret_cast<uint8_t *>(target));
			for (std::size_t column = 0; column < Size; ++column) {
				std::memcpy(bytes, glm::value_ptr(value[column]), element_size);
				bytes += alignment;
			}
		}
	};

	template<typename... Ts>
	struct primitive_type_information<std::tuple<Ts...>> {

	};

	template<typename T, std::size_t Size, std::size_t Alignment, std::size_t Columns>
	struct glm_mat_type_information {

		constexpr static std::size_t size = Columns * Alignment, alignment = Alignment;

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

	template<> struct primitive_type_information<glm::mat2>
		: glm_mat_type_information<glm::mat2, sizeof(float) * 2, sizeof(float) * 2, 2> {};
	template<> struct primitive_type_information<glm::mat2x3>
		: glm_mat_type_information<glm::mat2x3, sizeof(float) * 3, sizeof(float) * 4, 2> {};
	template<> struct primitive_type_information<glm::mat2x4>
		: glm_mat_type_information<glm::mat2x4, sizeof(float) * 4, sizeof(float) * 4, 2> {};
	template<> struct primitive_type_information<glm::mat3x2>
		: glm_mat_type_information<glm::mat3x2, sizeof(float) * 2, sizeof(float) * 2, 3> {};
	template<> struct primitive_type_information<glm::mat3>
		: glm_mat_type_information<glm::mat3, sizeof(float) * 3, sizeof(float) * 4, 3> {};
	template<> struct primitive_type_information<glm::mat3x4>
		: glm_mat_type_information<glm::mat3x4, sizeof(float) * 4, sizeof(float) * 4, 3> {};
	template<> struct primitive_type_information<glm::mat4>
		: glm_mat_type_information<glm::mat4, sizeof(float) * 4, sizeof(float) * 4, 4> {};
	template<> struct primitive_type_information<glm::mat4x2>
		: glm_mat_type_information<glm::mat4x2, sizeof(float) * 2, sizeof(float) * 2, 4> {};
	template<> struct primitive_type_information<glm::mat4x3>
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
