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
#ifndef QUEUE_H_
#define QUEUE_H_

#include <climits>
#include <vcc/command_buffer.h>
#include <vcc/device.h>
#include <vcc/fence.h>
#include <vcc/semaphore.h>
#include <vcc/surface.h>
#include <vcc/swapchain.h>

namespace vcc {
namespace queue {

struct queue_type : public internal::movable_with_parent<VkQueue, device::device_type> {
	friend VCC_LIBRARY queue_type get_device_queue(
		const type::supplier<device::device_type> &device,
		uint32_t queue_family_index, uint32_t queue_index);
	friend uint32_t get_family_index(queue_type &queue);

	queue_type() = default;
	queue_type(queue_type &&queue) = default;
	queue_type &operator=(const queue_type&) = delete;
	queue_type &operator=(queue_type&&copy) = default;

private:
	queue_type(VkQueue instance,
		const type::supplier<device::device_type> &parent, uint32_t family_index)
		: movable_with_parent(instance, parent),
		  family_index(family_index) {}
	uint32_t family_index;
};

VCC_LIBRARY queue_type get_device_queue(
	const type::supplier<device::device_type> &device,
	uint32_t queue_family_index, uint32_t queue_index);

VCC_LIBRARY queue_type get_queue(
		const type::supplier<device::device_type> &device, VkQueueFlags flags);
inline queue_type get_graphics_queue(
		const type::supplier<device::device_type> &device) {
	return get_queue(device, VK_QUEUE_GRAPHICS_BIT);
}
VCC_LIBRARY queue_type get_present_queue(
	const type::supplier<device::device_type> &device,
	surface::surface_type &surface);

// Helper method that returns two queues (possibly the same) where the first supports graphics and the other present.
// Tries to return the same queue twice if it finds one capable of both.
VCC_LIBRARY std::pair<queue_type, queue_type> get_graphics_and_present_queues(
	const type::supplier<device::device_type> &device,
	surface::surface_type &surface);

struct wait_semaphore {
	type::supplier<semaphore::semaphore_type> semaphore;
	VkPipelineStageFlags wait_dst_stage_mask;
};

// TODO(gardell): Support multiple submits
VCC_LIBRARY void submit(queue_type &queue,
	const std::vector<wait_semaphore> &wait_semaphores,
	const std::vector<type::supplier<command_buffer::command_buffer_type>> &command_buffers,
	const std::vector<type::supplier<semaphore::semaphore_type>> &signal_semaphores,
	const fence::fence_type &fence);

VCC_LIBRARY void submit(queue_type &queue,
	const std::vector<wait_semaphore> &wait_semaphores,
	const std::vector<type::supplier<command_buffer::command_buffer_type>> &command_buffers,
	const std::vector<type::supplier<semaphore::semaphore_type>> &signal_semaphores);

VCC_LIBRARY void wait_idle(queue_type &queue);

VCC_LIBRARY VkResult present(queue_type &queue,
	const std::vector<type::supplier<semaphore::semaphore_type>> &semaphores,
	const std::vector<type::supplier<swapchain::swapchain_type>> &swapchains,
	const std::vector<uint32_t> &image_indices);

inline uint32_t get_family_index(queue_type &queue) {
	return queue.family_index;
}

}  // namespace queue
}  // namespace vcc


#endif /* QUEUE_H_ */
