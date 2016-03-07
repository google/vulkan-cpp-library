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
#include <type/serialize.h>

namespace type {

std::size_t serialize_type::calculate_layout(memory_layout layout, std::size_t num_views,
		const std::size_t *sizes, const std::size_t *element_sizes,
		const std::size_t *base_alignments, std::size_t *offsets, std::size_t *strides) {

	const bool interleaved(internal::interleaved(layout));
	std::size_t size(0);

	if (interleaved) {
		// Find ranges of equal length that can be interleaved.
		typedef std::vector<std::pair<std::size_t, std::size_t>> range_container;
		range_container ranges;
		ranges.reserve(num_views);
		std::size_t previous_index = 0;
		for (std::size_t i = 1; i < num_views; ++i) {
			if (sizes[i] != sizes[previous_index]) {
				ranges.emplace_back(previous_index, i);
				previous_index = i;
			}
		}
		ranges.emplace_back(previous_index, num_views);

		offsets[0] = 0;
		for (const std::pair<std::size_t, std::size_t> &range : ranges) {
			std::size_t stride = 0;
			for (std::size_t i = range.first; i < range.second; ++i) {
				std::size_t padding(0);
				const std::size_t offset(size + stride);
				if (offset % base_alignments[i] && element_sizes[i] > base_alignments[i] - offset % base_alignments[i]) {
					padding = base_alignments[i] - offset % base_alignments[i];
				}
				offsets[i] = offset + padding;
				stride += element_sizes[i] + padding;
			}
			if (stride % internal::base_alignment) {
				stride += internal::base_alignment - stride % internal::base_alignment;
			}
			for (std::size_t i = range.first; i < range.second; ++i) {
				strides[i] = stride;
			}
			size += stride * sizes[range.first];
		}
	} else {
		offsets[0] = 0;
		size = sizes[0] * element_sizes[0];
		strides[0] = element_sizes[0];
		for (std::size_t i = 1; i < num_views; ++i) {
			const std::size_t previous_byte_size(sizes[i - 1] * element_sizes[i - 1]);
			const std::size_t byte_size(sizes[i] * element_sizes[i]);
			const std::size_t offset = offsets[i - 1] + previous_byte_size;
			std::size_t padding = 0;
			const std::size_t alignment(offset % base_alignments[i]);
			if (alignment != 0 && byte_size > base_alignments[i] - alignment) {
				padding = base_alignments[i] - offset % base_alignments[i];
			}
			offsets[i] = offset + padding;
			size += sizes[i] * element_sizes[i] + padding;
			strides[i] = element_sizes[i];
		}
	}
	return size;
}

void flush(const serialize_type &serialize, void *output) {
	for (const std::unique_ptr<internal::adapter> &adapter : serialize.adapters) {
		adapter->copy(output);
	}
}

bool dirty(const serialize_type &serialize) {
	return std::find_if(serialize.adapters.begin(), serialize.adapters.end(),
			[](const serialize_type::adapter_container_type::value_type &adapter) {
		return adapter->dirty();
	}) != serialize.adapters.end();
}

std::size_t size(const serialize_type &serialize) {
	return serialize.size;
}

memory_layout layout(const serialize_type &serialize) {
	return serialize.layout;
}

}  // namespace type
