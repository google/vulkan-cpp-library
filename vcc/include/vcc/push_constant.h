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
#ifndef VCC_PUSH_CONSTANT_H_
#define VCC_PUSH_CONSTANT_H_

#include <vcc/pipeline_layout.h>
#include <type/serialize.h>

namespace vcc {
namespace pipeline_layout {

namespace internal {

VCC_LIBRARY void flush(const type::supplier<type::serialize_type> &constants,
	VkPipelineLayout pipeline_layout,
	const std::vector<VkPushConstantRange> &push_constant_ranges,
	queue::queue_type &queue);

}  // namespace internal

template<typename... StorageType>
pipeline_layout_type create(const type::supplier<device::device_type> &device,
	const std::vector<type::supplier<descriptor_set_layout::descriptor_set_layout_type>> &set_layouts,
	const std::vector<VkPushConstantRange> &push_constant_ranges, type::memory_layout layout, StorageType... storages) {
	pipeline_layout_type pipeline_layout(create(type::supplier<device::device_type>(device), set_layouts, push_constant_ranges));
	pipeline_layout::internal::get_pre_execute_callbacks(pipeline_layout).add(std::bind(&internal::flush,
		type::supplier<type::serialize_type>(type::make_serialize(layout, std::forward<StorageType>(storages)...)),
		vcc::internal::get_instance(pipeline_layout), push_constant_ranges, std::placeholders::_1));
	return std::move(pipeline_layout);
}

}  // namespace pipeline_layout
}  // namespace vcc

#endif // VCC_PUSH_CONSTANT_H_