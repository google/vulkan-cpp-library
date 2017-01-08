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
#include <inttypes.h>
#include <type/serialize.h>
#include <type/storage.h>

namespace type {

namespace internal {

	template<>
	struct copy_type<glm::mat3> {
		typedef glm::mat3 value_type;

		static void copy(const value_type &value, void *destination) {
			const uint32_t *array = (const uint32_t *)&value;
			uint32_t *target = (uint32_t *)destination;
			for (int row = 0; row < 3; ++row) {
				for (int col = 0; col < 3; ++col) {
					target[row * 4 + col] = array[row * 3 + col];
				}
			}
		}
	};

}  // namespace internal

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

}  // namespace internal

template<typename T>
struct type_information {};

template<>
struct type_information<float_array>
	: public internal::type_information_traits<type_float, 1, 1, true> {};
template<>
struct type_information<vec2_array>
	: public internal::type_information_traits<type_float, 2, 1, true> {};
template<>
struct type_information<vec3_array>
	: public internal::type_information_traits<type_float, 3, 1, true> {};
template<>
struct type_information<vec4_array>
	: public internal::type_information_traits<type_float, 4, 1, true> {};
template<>
struct type_information<int_array>
	: public internal::type_information_traits<type_int, 1, 1, true> {};
template<>
struct type_information<ivec2_array>
	: public internal::type_information_traits<type_int, 2, 1, true> {};
template<>
struct type_information<ivec3_array>
	: public internal::type_information_traits<type_int, 3, 1, true> {};
template<>
struct type_information<ivec4_array>
	: public internal::type_information_traits<type_int, 4, 1, true> {};
template<>
struct type_information<mat2_array>
	: public internal::type_information_traits<type_float, 2, 2, true> {};
template<>
struct type_information<mat3_array>
	: public internal::type_information_traits<type_float, 3, 3, true> {};
template<>
struct type_information<mat4_array>
	: public internal::type_information_traits<type_float, 4, 4, true> {};
template<>
struct type_information<ubyte_array>
	: public internal::type_information_traits<type_ubyte, 1, 1, true> {};
template<>
struct type_information<ushort_array>
	: public internal::type_information_traits<type_ushort, 1, 1, true> {};
template<>
struct type_information<uint_array>
	: public internal::type_information_traits<type_uint, 1, 1, true> {};

template<>
struct type_information<float_type>
	: public internal::type_information_traits<type_float, 1, 1, false> {};
template<>
struct type_information<vec2>
	: public internal::type_information_traits<type_float, 2, 1, false> {};
template<>
struct type_information<vec3>
	: public internal::type_information_traits<type_float, 3, 1, false> {};
template<>
struct type_information<vec4>
	: public internal::type_information_traits<type_float, 4, 1, false> {};
template<>
struct type_information<int>
	: public internal::type_information_traits<type_int, 1, 1, false> {};
template<>
struct type_information<ivec2>
	: public internal::type_information_traits<type_int, 2, 1, false> {};
template<>
struct type_information<ivec3>
	: public internal::type_information_traits<type_int, 3, 1, false> {};
template<>
struct type_information<ivec4>
	: public internal::type_information_traits<type_int, 4, 1, false> {};
template<>
struct type_information<mat2>
	: public internal::type_information_traits<type_float, 2, 2, false> {};
template<>
struct type_information<mat3>
	: public internal::type_information_traits<type_float, 3, 3, false> {};
template<>
struct type_information<mat4>
	: public internal::type_information_traits<type_float, 4, 4, false> {};

template<>
struct type_information<const_float_array>
	: public type_information<float_array> {};
template<>
struct type_information<const_vec2_array>
	: public type_information<vec2_array> {};
template<>
struct type_information<const_vec3_array>
	: public type_information<vec3_array> {};
template<>
struct type_information<const_vec4_array>
	: public type_information<vec4_array> {};
template<>
struct type_information<const_int_array>
	: public type_information<int_array> {};
template<>
struct type_information<const_ivec2_array>
	: public type_information<ivec2_array> {};
template<>
struct type_information<const_ivec3_array>
	: public type_information<ivec3_array> {};
template<>
struct type_information<const_ivec4_array>
	: public type_information<ivec4_array> {};
template<>
struct type_information<const_mat2_array>
	: public type_information<mat2_array> {};
template<>
struct type_information<const_mat3_array>
	: public type_information<mat3_array> {};
template<>
struct type_information<const_mat4_array>
	: public type_information<mat4_array> {};
template<>
struct type_information<const_ubyte_array>
	: public type_information<ubyte_array> {};
template<>
struct type_information<const_ushort_array>
	: public type_information<ushort_array> {};
template<>
struct type_information<const_float_type>
	: public type_information<float_type> {};
template<>
struct type_information<const_vec2>
	: public type_information<vec2> {};
template<>
struct type_information<const_vec3>
	: public type_information<vec3> {};
template<>
struct type_information<const_vec4>
	: public type_information<vec4> {};
template<>
struct type_information<const_ivec2>
	: public type_information<ivec2> {};
template<>
struct type_information<const_ivec3>
	: public type_information<ivec3> {};
template<>
struct type_information<const_ivec4>
	: public type_information<ivec4> {};
template<>
struct type_information<const_mat2>
	: public type_information<mat2> {};
template<>
struct type_information<const_mat3>
	: public type_information<mat3> {};
template<>
struct type_information<const_mat4>
	: public type_information<mat4> {};
template<>
struct type_information<writable_float_array>
	: public type_information<float_array> {};
template<>
struct type_information<writable_vec2_array>
	: public type_information<vec2_array> {};
template<>
struct type_information<writable_vec3_array>
	: public type_information<vec3_array> {};
template<>
struct type_information<writable_vec4_array>
	: public type_information<vec4_array> {};
template<>
struct type_information<writable_int_array>
	: public type_information<int_array> {};
template<>
struct type_information<writable_ivec2_array>
	: public type_information<ivec2_array> {};
template<>
struct type_information<writable_ivec3_array>
	: public type_information<ivec3_array> {};
template<>
struct type_information<writable_ivec4_array>
	: public type_information<ivec4_array> {};
template<>
struct type_information<writable_mat2_array>
	: public type_information<mat2_array> {};
template<>
struct type_information<writable_mat3_array>
	: public type_information<mat3_array> {};
template<>
struct type_information<writable_mat4_array>
	: public type_information<mat4_array> {};
template<>
struct type_information<writable_ubyte_array>
	: public type_information<ubyte_array> {};
template<>
struct type_information<writable_ushort_array>
	: public type_information<ushort_array> {};

}  // namespace type

#endif // DATA_TYPES_H_
