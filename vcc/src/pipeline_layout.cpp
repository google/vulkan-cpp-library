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
#define NOMINMAX
#include <algorithm>
#include <cassert>
#include <iterator>
#include <vcc/command_buffer.h>
#include <vcc/queue.h>
#include <vcc/pipeline_layout.h>

namespace vcc {
namespace pipeline_layout {

pipeline_layout_type create(const type::supplier<device::device_type> &device,
		const std::vector<type::supplier<vcc::descriptor_set_layout::descriptor_set_layout_type>> &set_layouts,
		const std::vector<VkPushConstantRange> &push_constant_ranges) {
	VkPipelineLayoutCreateInfo create = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, NULL, 0};
	create.setLayoutCount = (uint32_t) set_layouts.size();
	std::vector<VkDescriptorSetLayout> converted_set_layouts;
	converted_set_layouts.reserve(set_layouts.size());
	std::transform(set_layouts.begin(), set_layouts.end(), std::back_inserter(converted_set_layouts),
			[](const type::supplier<vcc::descriptor_set_layout::descriptor_set_layout_type> &set_layout){
		return vcc::internal::get_instance(*set_layout);
	});
	create.pSetLayouts = set_layouts.empty() ? NULL : &converted_set_layouts.front();
	create.pushConstantRangeCount = (uint32_t) push_constant_ranges.size();
	create.pPushConstantRanges = push_constant_ranges.empty() ? NULL : &push_constant_ranges.front();
	VkPipelineLayout pipeline_layout;
	VKCHECK(vkCreatePipelineLayout(vcc::internal::get_instance(*device), &create, NULL, &pipeline_layout));
	return pipeline_layout_type(pipeline_layout, device, set_layouts);
}

namespace internal {

void flush(const type::supplier<type::serialize_type> &constants,
	VkPipelineLayout pipeline_layout,
	const std::vector<VkPushConstantRange> &push_constant_ranges,
	queue::queue_type &queue) {
	if (type::dirty(*constants)) {
		const type::supplier<device::device_type> device(
			vcc::internal::get_parent(queue));
		std::string buffer(type::size(*constants), '\0');
		type::flush(*constants, &buffer[0]);
		command_pool::command_pool_type command_pool(command_pool::create(
			device, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, queue::get_family_index(queue)));
		command_buffer::command_buffer_type cmd(std::move(
			command_buffer::allocate(device, std::ref(command_pool),
				VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1).front()));
		{
			command_buffer::begin_type begin(command_buffer::begin(
				std::ref(cmd), VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
				VK_FALSE, 0, 0));
			for (const VkPushConstantRange &range : push_constant_ranges) {
				assert(range.offset + range.size <= buffer.size());
				vkCmdPushConstants(vcc::internal::get_instance(cmd), pipeline_layout,
					range.stageFlags, range.offset, range.size,
					&buffer[range.offset]);
			}
		}
		// Must block until our command finish executing.
		fence::fence_type fence(fence::create(device));
		queue::submit(queue, {}, { std::ref(cmd) }, {}, fence);
		fence::wait(*device, { std::ref(fence) }, true,
			std::chrono::nanoseconds::max());
	}
}

}  // namespace internal

}  // namespace pipeline_layout
}  // namespace vcc


