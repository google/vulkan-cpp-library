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
#include <iterator>
#include <vcc/command.h>
#include <vcc/command_buffer.h>

namespace vcc {
namespace command_buffer {

std::vector<command_buffer_type> allocate(
		const type::supplier<device::device_type> &device,
		const type::supplier<command_pool::command_pool_type> &command_pool,
		VkCommandBufferLevel level, uint32_t commandBufferCount) {
	std::vector<VkCommandBuffer> buffers(commandBufferCount);
	VkCommandBufferAllocateInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, NULL};
	info.level = level;
	info.commandBufferCount = commandBufferCount;
	{
		std::lock_guard<std::mutex> command_pool_lock(
			vcc::internal::get_mutex(*command_pool));
		info.commandPool = vcc::internal::get_instance(*command_pool);
		VKCHECK(vkAllocateCommandBuffers(vcc::internal::get_instance(*device),
			&info, buffers.data()));
	}
	std::vector<command_buffer_type> command_buffers;
	command_buffers.reserve(commandBufferCount);
	for (VkCommandBuffer buffer : buffers) {
		command_buffers.push_back(command_buffer_type(buffer, command_pool, device));
	}
	return std::move(command_buffers);
}

begin_type::~begin_type() {
	VKCHECK(vkEndCommandBuffer(vcc::internal::get_instance(*command_buffer)));
}

begin_type::begin_type(
	const type::supplier<command_buffer_type> &command_buffer)
	: command_buffer(command_buffer),
	  command_buffer_lock(vcc::internal::get_mutex(*command_buffer)) {}

begin_type begin(const type::supplier<command_buffer_type> &command_buffer,
	VkCommandBufferUsageFlags flags,
	const type::supplier<render_pass::render_pass_type> &render_pass,
	uint32_t subpass,
	const type::supplier<framebuffer::framebuffer_type> &framebuffer,
	VkBool32 occlusionQueryEnable, VkQueryControlFlags queryFlags,
	VkQueryPipelineStatisticFlags pipelineStatistics) {
	VkCommandBufferInheritanceInfo inheritance_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, NULL };
	inheritance_info.renderPass = VkRenderPass(vcc::internal::get_instance(*render_pass));
	inheritance_info.subpass = subpass;
	inheritance_info.framebuffer = VkFramebuffer(vcc::internal::get_instance(*framebuffer));
	inheritance_info.occlusionQueryEnable = occlusionQueryEnable;
	inheritance_info.queryFlags = queryFlags;
	inheritance_info.pipelineStatistics = pipelineStatistics;
	VkCommandBufferBeginInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL};
	info.flags = flags;
	info.pInheritanceInfo = &inheritance_info;
	VKCHECK(vkBeginCommandBuffer(vcc::internal::get_instance(*command_buffer), &info));
	return begin_type(command_buffer);
}

begin_type begin(const type::supplier<command_buffer_type> &command_buffer,
	VkCommandBufferUsageFlags flags,
	VkBool32 occlusionQueryEnable, VkQueryControlFlags queryFlags,
	VkQueryPipelineStatisticFlags pipelineStatistics) {
	VkCommandBufferInheritanceInfo inheritance_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, NULL };
	inheritance_info.renderPass = VK_NULL_HANDLE;
	inheritance_info.subpass = 0;
	inheritance_info.framebuffer = VK_NULL_HANDLE;
	inheritance_info.occlusionQueryEnable = occlusionQueryEnable;
	inheritance_info.queryFlags = queryFlags;
	inheritance_info.pipelineStatistics = pipelineStatistics;
	VkCommandBufferBeginInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL };
	info.flags = flags;
	info.pInheritanceInfo = &inheritance_info;
	VKCHECK(vkBeginCommandBuffer(vcc::internal::get_instance(*command_buffer), &info));
	return begin_type(command_buffer);
}

}  // namespace command_buffer
}  // namespace vcc


