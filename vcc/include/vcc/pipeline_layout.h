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
#ifndef PIPELINE_LAYOUT_H_
#define PIPELINE_LAYOUT_H_

#include <type/serialize.h>
#include <vcc/device.h>
#include <vcc/descriptor_set_layout.h>
#include <vcc/internal/hook.h>

namespace vcc {
namespace queue {

struct queue_type;

}  // namespace queue

namespace pipeline_layout {

namespace internal {

template<typename PipelineLayoutT>
auto get_pre_execute_callbacks(PipelineLayoutT &layout)
		->decltype(layout.pre_execute_callbacks)& {
	return layout.pre_execute_callbacks;
}

template<typename PipelineLayoutT>
auto get_set_layouts(const PipelineLayoutT &pipeline)
		->const decltype(pipeline.set_layouts)& {
	return pipeline.set_layouts;
}

}  // namespace internal

struct pipeline_layout_type
	: vcc::internal::movable_destructible_with_parent<VkPipelineLayout,
		device::device_type, vkDestroyPipelineLayout> {
	template<typename PipelineLayoutT>
	friend auto internal::get_pre_execute_callbacks(PipelineLayoutT &layout)
		->decltype(layout.pre_execute_callbacks)&;
	friend VCC_LIBRARY pipeline_layout_type create(
		const type::supplier<device::device_type> &device,
		const std::vector<type::supplier<vcc::descriptor_set_layout::descriptor_set_layout_type>> &set_layouts,
		const std::vector<VkPushConstantRange> &push_constant_ranges);
	template<typename PipelineLayoutT>
	friend auto internal::get_set_layouts(const PipelineLayoutT &pipeline)
		->const decltype(pipeline->set_layouts)&;

	pipeline_layout_type() = default;
	pipeline_layout_type(const pipeline_layout_type &) = delete;
	pipeline_layout_type(pipeline_layout_type &&) = default;
	pipeline_layout_type &operator=(const pipeline_layout_type &) = delete;
	pipeline_layout_type &operator=(pipeline_layout_type &&) = default;

private:
	pipeline_layout_type(VkPipelineLayout instance,
		const type::supplier<device::device_type> &parent,
		const std::vector<type::supplier<vcc::descriptor_set_layout::descriptor_set_layout_type>> &set_layouts)
		: vcc::internal::movable_destructible_with_parent<VkPipelineLayout,
				device::device_type, vkDestroyPipelineLayout>(
			instance, type::supplier<device::device_type>(parent)),
			set_layouts(set_layouts) {}
	std::vector<type::supplier<vcc::descriptor_set_layout::descriptor_set_layout_type>> set_layouts;
	vcc::internal::hook_container_type<queue::queue_type &> pre_execute_callbacks;
};

VCC_LIBRARY pipeline_layout_type create(const type::supplier<device::device_type> &device,
	const std::vector<type::supplier<vcc::descriptor_set_layout::descriptor_set_layout_type>> &set_layouts,
	const std::vector<VkPushConstantRange> &push_constant_ranges = {});

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

#endif /* PIPELINE_LAYOUT_H_ */
