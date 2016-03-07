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
#include <cassert>
#include <vcc/command_buffer.h>
#include <vcc/push_constant.h>
#include <vcc/queue.h>

namespace vcc {
namespace pipeline_layout {

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
