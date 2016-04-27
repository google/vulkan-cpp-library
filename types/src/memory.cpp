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
#include <algorithm>
#include <cassert>
#include <type/memory.h>

namespace type {
namespace internal {

const std::size_t base_alignment(4 * sizeof(float));

std::size_t calculate_element_size(memory_layout layout, std::size_t type_size, bool array) {
	switch (layout) {
	case linear:
		return type_size;
	case interleaved_std140:
		return std::max(base_alignment, type_size);
	case interleaved_std430:
		return type_size == 12 ? base_alignment : type_size;
	case linear_std140:
		return array && type_size < base_alignment ? base_alignment : type_size;
	case linear_std430:
		return array && type_size == 12 ? base_alignment : type_size;
	default:
		assert(!"Unsupported layout");
		return 0;
	}
}

std::size_t calculate_base_alignment(memory_layout layout, std::size_t type_size, bool array) {
	switch (layout) {
	case linear:
		return 1;
	case interleaved_std430:
	case linear_std430:
		return 4;
	case interleaved_std140:
		return base_alignment;
	case linear_std140:
		return array ? base_alignment : (type_size == 12 ? base_alignment : type_size);
	default:
		assert(!"Unsupported layout");
		return 0;
	}
}

bool interleaved(memory_layout layout) {
	switch (layout) {
	case interleaved_std140:
	case interleaved_std430:
		return true;
	case linear:
	case linear_std140:
	case linear_std430:
		return false;
	default:
		assert(!"Unsupported layout");
		return 0;
	}
}

}  // namespace internal
}  // namespace type
