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

#include <vcc/device.h>
#include <vcc/descriptor_set_layout.h>
#include <vcc/internal/hook.h>

namespace vcc {
namespace queue {

struct queue_type;

}  // namespace queue

namespace pipeline_layout {

struct pipeline_layout_type;

namespace internal {
VCC_LIBRARY vcc::internal::hook_container_type<queue::queue_type &>
	&get_pre_execute_callbacks(pipeline_layout_type &);
}  // namespace internal

struct pipeline_layout_type
	: vcc::internal::movable_destructible_with_parent<VkPipelineLayout,
		device::device_type, vkDestroyPipelineLayout> {
	friend vcc::internal::hook_container_type<queue::queue_type &>
		&internal::get_pre_execute_callbacks(pipeline_layout_type &);
	friend VCC_LIBRARY pipeline_layout_type create(
		const type::supplier<device::device_type> &device,
		const std::vector<type::supplier<vcc::descriptor_set_layout::descriptor_set_layout_type>> &set_layouts,
		const std::vector<VkPushConstantRange> &push_constant_ranges);

	pipeline_layout_type() = default;
	pipeline_layout_type(const pipeline_layout_type &) = delete;
	pipeline_layout_type(pipeline_layout_type &&) = default;
	pipeline_layout_type &operator=(const pipeline_layout_type &) = delete;
	pipeline_layout_type &operator=(pipeline_layout_type &&) = default;

private:
	pipeline_layout_type(VkPipelineLayout instance, const type::supplier<device::device_type> &parent)
		: vcc::internal::movable_destructible_with_parent<VkPipelineLayout,
				device::device_type, vkDestroyPipelineLayout>(
			instance, type::supplier<device::device_type>(parent)) {}
	vcc::internal::hook_container_type<queue::queue_type &> pre_execute_callbacks;
};

VCC_LIBRARY pipeline_layout_type create(const type::supplier<device::device_type> &device,
	const std::vector<type::supplier<vcc::descriptor_set_layout::descriptor_set_layout_type>> &set_layouts,
	const std::vector<VkPushConstantRange> &push_constant_ranges = {});

}  // namespace pipeline_layout
}  // namespace vcc

#endif /* PIPELINE_LAYOUT_H_ */
