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
#ifndef COMMAND_BUFFER_H_
#define COMMAND_BUFFER_H_

#include <array>
#include <vcc/buffer.h>
#include <vcc/command_pool.h>
#include <vcc/descriptor_set.h>
#include <vcc/event.h>
#include <vcc/framebuffer.h>
#include <vcc/internal/hook.h>
#include <vcc/pipeline.h>
#include <vcc/query_pool.h>
#include <vcc/render_pass.h>
#include <vcc/util.h>

namespace vcc {
namespace queue {

struct queue_type;

}  // namespace queue

namespace command {

struct build_type;

}  // namespace command

namespace command_buffer {
namespace internal {

template<typename T>
vcc::internal::hook_container_type<const queue::queue_type &> &get_pre_execute_hook(T &value) {
	return value.pre_execute_hook;
}

template<typename T>
const vcc::internal::hook_container_type<const queue::queue_type &> &get_pre_execute_hook(
		const T &value) {
	return value.pre_execute_hook;
}

}  // namespace internal

struct command_buffer_type
	: public vcc::internal::movable_allocated_with_pool_parent1<VkCommandBuffer,
		const device::device_type, const command_pool::command_pool_type,
		vkFreeCommandBuffers> {
	friend VCC_LIBRARY std::vector<command_buffer_type> allocate(
		const type::supplier<const device::device_type> &device,
		const type::supplier<const command_pool::command_pool_type> &command_pool,
		VkCommandBufferLevel level, uint32_t commandBufferCount);
	template<typename... CommandsT>
	friend void compile(command_buffer_type &command_buffer,
		VkCommandBufferUsageFlags flags,
		const type::supplier<const render_pass::render_pass_type> &render_pass,
		uint32_t subpass,
		const type::supplier<const framebuffer::framebuffer_type> &framebuffer,
		VkBool32 occlusionQueryEnable, VkQueryControlFlags queryFlags,
		VkQueryPipelineStatisticFlags pipelineStatistics,
		CommandsT&&... commands);
	template<typename... CommandsT>
	friend void compile(command_buffer_type &command_buffer,
		VkCommandBufferUsageFlags flags,
		VkBool32 occlusionQueryEnable, VkQueryControlFlags queryFlags,
		VkQueryPipelineStatisticFlags pipelineStatistics,
		CommandsT&&... commands);
	template<typename T>
	friend vcc::internal::hook_container_type<const queue::queue_type &>
		&internal::get_pre_execute_hook(T &value);
	template<typename T>
	friend const vcc::internal::hook_container_type<const queue::queue_type &>
		&internal::get_pre_execute_hook(const T &value);
	friend struct command::build_type;

	command_buffer_type() = default;
	command_buffer_type(command_buffer_type &&) = default;
	command_buffer_type &operator=(command_buffer_type &&) = default;
	command_buffer_type &operator=(const command_buffer_type &) = default;

private:
	command_buffer_type(VkCommandBuffer instance,
		const type::supplier<const command_pool::command_pool_type> &pool,
		const type::supplier<const device::device_type> &parent)
		: movable_allocated_with_pool_parent1(instance, pool, parent) {}

	vcc::internal::hook_container_type<const queue::queue_type&> pre_execute_hook;
	vcc::internal::reference_container_type references;
};

VCC_LIBRARY std::vector<command_buffer_type> allocate(
	const type::supplier<const device::device_type> &device,
	const type::supplier<const command_pool::command_pool_type> &command_pool,
	VkCommandBufferLevel level, uint32_t commandBufferCount);

}  // namespace command_buffer
}  // namespace vcc

#endif /* COMMAND_BUFFER_H_ */
