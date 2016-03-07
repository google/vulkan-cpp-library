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
#include <limits>
#include <vcc/physical_device.h>
#include <vcc/queue.h>

namespace vcc {
namespace queue {

queue_type get_device_queue(const type::supplier<device::device_type> &device,
		uint32_t queue_family_index, uint32_t queue_index) {
	VkQueue queue;
	vkGetDeviceQueue(internal::get_instance(*device), queue_family_index,
		queue_index, &queue);
	return queue_type(queue, device, queue_family_index);
}

queue_type get_graphics_queue(const type::supplier<device::device_type> &device) {
	const std::vector<VkQueueFamilyProperties> queue_props(
		physical_device::queue_famility_properties(device::get_physical_device(*device)));
	for (std::size_t i = 0; i < queue_props.size(); ++i) {
		if (queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			return get_device_queue(device, (uint32_t)i, 0);
		}
	}
	throw vcc_exception("Failed to find a graphics queue");
}

queue_type get_present_queue(const type::supplier<device::device_type> &device,
		surface::surface_type &surface) {
	const std::vector<VkQueueFamilyProperties> queue_props(
		physical_device::queue_famility_properties(
			device::get_physical_device(*device)));
	for (std::size_t i = 0; i < queue_props.size(); ++i) {
		if (vcc::surface::physical_device_support(device::get_physical_device(*device),
				surface, (uint32_t) i)) {
			get_device_queue(type::supplier<device::device_type>(device),
				(uint32_t) i, 0);
		}
	}
	throw vcc_exception("Failed to find a present queue");
}

std::pair<queue_type, queue_type> get_graphics_and_present_queues(
		const type::supplier<device::device_type> &device,
		surface::surface_type &surface) {
	const std::vector<VkQueueFamilyProperties> queue_props(
		physical_device::queue_famility_properties(device::get_physical_device(*device)));
	std::size_t graphics_index = std::numeric_limits<std::size_t>::max();
	std::size_t present_index = std::numeric_limits<std::size_t>::max();
	for (std::size_t i = 0; i < queue_props.size(); ++i) {
		const bool present_support(vcc::surface::physical_device_support(
			device::get_physical_device(*device), surface, (uint32_t) i));
		if (present_support) {
			present_index = i;
		}
		if (queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphics_index = i;
			if (present_support) {
				present_index = i;
				break;
			}
		}
	}
	if (present_index == std::numeric_limits<std::size_t>::max()
		|| graphics_index == std::numeric_limits<std::size_t>::max()) {
		throw vcc_exception("failed to find queues for present and graphics");
	}
	return std::make_pair(get_device_queue(device, (uint32_t) graphics_index, 0),
		get_device_queue(device, (uint32_t) present_index, 0));
}

inline void submit(queue_type &queue,
		const std::vector<wait_semaphore> &wait_semaphores,
		const std::vector<type::supplier<command_buffer::command_buffer_type>> &command_buffers,
		const std::vector<type::supplier<semaphore::semaphore_type>> &signal_semaphores,
		VkFence fence) {
	std::vector<VkCommandBuffer> converted_command_buffers;
	converted_command_buffers.reserve(command_buffers.size());
	for (const type::supplier<command_buffer::command_buffer_type> &command_buffer : command_buffers) {
		converted_command_buffers.push_back(internal::get_instance(*command_buffer));
		command_buffer::internal::get_pre_execute_hook(*command_buffer)(queue);
	}
	std::vector<VkSemaphore> converted_wait_semaphores;
	converted_wait_semaphores.reserve(wait_semaphores.size());
	std::vector<VkPipelineStageFlags> wait_mask;
	wait_mask.reserve(wait_semaphores.size());
	for (const wait_semaphore &semaphore : wait_semaphores) {
		converted_wait_semaphores.push_back(internal::get_instance(*semaphore.semaphore));
		wait_mask.push_back(semaphore.wait_dst_stage_mask);
	}
	std::vector<VkSemaphore> converted_signal_semaphores;
	converted_signal_semaphores.reserve(signal_semaphores.size());
	for (const type::supplier<semaphore::semaphore_type> &semaphore : signal_semaphores) {
		converted_signal_semaphores.push_back(internal::get_instance(*semaphore));
	}
	VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL };
	submit.waitSemaphoreCount = (uint32_t)converted_wait_semaphores.size();
	submit.pWaitSemaphores = converted_wait_semaphores.empty() ? NULL : &converted_wait_semaphores.front();
	//submit.pWaitDstStageMask = wait_mask.empty() ? NULL : &wait_mask.front();
	submit.commandBufferCount = (uint32_t)converted_command_buffers.size();
	submit.pCommandBuffers = converted_command_buffers.data();
	submit.signalSemaphoreCount = (uint32_t)converted_signal_semaphores.size();
	submit.pSignalSemaphores = converted_signal_semaphores.empty() ? NULL : &converted_signal_semaphores.front();
	VKCHECK(vkQueueSubmit(internal::get_instance(queue), 1, &submit, fence));
}

void submit(queue_type &queue,
		const std::vector<wait_semaphore> &wait_semaphores,
		const std::vector<type::supplier<command_buffer::command_buffer_type>> &command_buffers,
		const std::vector<type::supplier<semaphore::semaphore_type>> &signal_semaphores,
		const fence::fence_type &fence) {
	submit(queue, wait_semaphores, command_buffers, signal_semaphores, internal::get_instance(fence));
}

void submit(queue_type &queue,
	const std::vector<wait_semaphore> &wait_semaphores,
	const std::vector<type::supplier<command_buffer::command_buffer_type>> &command_buffers,
	const std::vector<type::supplier<semaphore::semaphore_type>> &signal_semaphores) {
	submit(queue, wait_semaphores, command_buffers, signal_semaphores, VK_NULL_HANDLE);
}

void wait_idle(queue_type &queue) {
	VKCHECK(vkQueueWaitIdle(internal::get_instance(queue)));
}

VkResult present(queue_type &queue, const std::vector<type::supplier<semaphore::semaphore_type>> &semaphores, const std::vector<type::supplier<swapchain::swapchain_type>> &swapchains, const std::vector<uint32_t> &image_indices) {
	VkPresentInfoKHR info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, NULL};
	info.waitSemaphoreCount = (uint32_t) semaphores.size();
	std::vector<VkSemaphore> converted_semaphores;
	converted_semaphores.reserve(semaphores.size());
	for (const type::supplier<semaphore::semaphore_type> &semaphore : semaphores) {
		converted_semaphores.push_back(internal::get_instance(*semaphore));
	}
	info.pWaitSemaphores = semaphores.empty() ? NULL : &converted_semaphores.front();
	info.swapchainCount = (uint32_t) swapchains.size();
	std::vector<VkSwapchainKHR> converted_swapchains;
	converted_swapchains.reserve(swapchains.size());
	for (const type::supplier<swapchain::swapchain_type> &swapchain : swapchains) {
		converted_swapchains.push_back(internal::get_instance(*swapchain));
	}
	info.pSwapchains = swapchains.empty() ? NULL : &converted_swapchains.front();
	info.pImageIndices = image_indices.empty() ? NULL : &image_indices.front();
	info.pResults = NULL;
	return vkQueuePresentKHR(internal::get_instance(queue), &info);
}

}  // namespace queue
}  // namespace vcc
