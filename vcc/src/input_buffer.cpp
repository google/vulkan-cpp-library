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
#include <vcc/command.h>
#include <vcc/command_buffer.h>
#include <vcc/input_buffer.h>
#include <vcc/memory.h>
#include <vcc/queue.h>

namespace vcc {
namespace input_buffer {

bool flush(const input_buffer_type &buffer) {
	if (type::dirty(buffer.serialize)) {
		std::unique_lock<std::mutex> lock(buffer.mutex);
		if (type::dirty(buffer.serialize)) {
			const memory::map_type map(memory::map(
				vcc::internal::get_memory(buffer.buffer),
				vcc::internal::get_offset(buffer.buffer),
				type::size(buffer.serialize)));
			type::flush(buffer.serialize, (uint8_t *)map.data);
			return true;
		}
	}
	return false;
}

bool flush(const queue::queue_type &queue, const input_buffer_type &buffer) {
	if (flush(buffer)) {
		std::unique_lock<std::mutex> lock(buffer.mutex);
		type::supplier<const device::device_type> device(vcc::internal::get_parent(buffer.buffer));
		command_pool::command_pool_type command_pool(command_pool::create(device,
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, queue::get_family_index(queue)));
		command_buffer::command_buffer_type command_buffer(std::move(command_buffer::allocate(
			device, std::ref(command_pool), VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1).front()));
		command::compile(
			vcc::command::build(std::ref(command_buffer),
				VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, VK_FALSE, 0, 0),
			command::pipeline_barrier{ VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, {},
				{
					command::buffer_memory_barrier_type{
						VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
						VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
						std::ref(buffer.buffer) }
				}, {}
			});
		// Must block until our command finish executing.
		fence::fence_type fence(vcc::fence::create(device));
		queue::submit(queue, {}, { command_buffer }, {}, fence);
		fence::wait(*device, { fence }, true);
		return true;
	} else {
		return false;
	}
}

}  // namespace input_buffer
}  // namespace vcc
