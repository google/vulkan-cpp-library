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
#ifndef _VCC_COMMAND_H_
#define _VCC_COMMAND_H_

#include <array>
#include <vcc/command_buffer.h>
#include <vcc/descriptor_set.h>
#include <vcc/event.h>
#include <vcc/input_buffer.h>
#include <vcc/pipeline.h>
#include <vcc/query_pool.h>

namespace vcc {
namespace command {

struct bind_pipeline {
	VkPipelineBindPoint pipelineBindPoint;
	type::supplier<pipeline::pipeline_type> pipeline;
};

struct set_viewport {
	uint32_t first_viewport;
	std::vector<VkViewport> viewports;
};

struct set_scissor {
	uint32_t first_scissor;
	std::vector<VkRect2D> scissors;
};

struct set_line_width {
	float lineWidth;
};

struct set_depth_bias {
	float depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor;
};

struct set_blend_constants {
	std::array<float, 4> blendConstants;
};

struct set_depth_bounds {
	float minDepthBounds, maxDepthBounds;
};

struct set_stencil_compare_mask {
	VkStencilFaceFlags faceMask;
	uint32_t compareMask;
};

struct set_stencil_write_mask {
	VkStencilFaceFlags faceMask;
	uint32_t writeMask;
};

struct set_stencil_reference {
	VkStencilFaceFlags faceMask;
	uint32_t reference;
};

struct bind_descriptor_sets {
	VkPipelineBindPoint pipelineBindPoint;
	type::supplier<pipeline_layout::pipeline_layout_type> layout;
	uint32_t firstSet;
	std::vector<type::supplier<descriptor_set::descriptor_set_type>>
		descriptor_sets;
	std::vector<uint32_t> dynamic_offsets;
};

struct bind_index_buffer_type {
	type::supplier<buffer::buffer_type> buffer;
	VkDeviceSize offset;
	VkIndexType indexType;
};

inline bind_index_buffer_type bind_index_buffer(
	const type::supplier<buffer::buffer_type> &buffer,
	VkDeviceSize offset, VkIndexType indexType) {
	return bind_index_buffer_type{ buffer, offset, indexType };
}

struct bind_index_data_buffer_type {
	type::supplier<input_buffer::input_buffer_type> buffer;
	VkDeviceSize offset;
	VkIndexType indexType;
};

inline bind_index_data_buffer_type bind_index_data_buffer(
	const type::supplier<input_buffer::input_buffer_type> &buffer,
	VkDeviceSize offset, VkIndexType indexType) {
	return bind_index_data_buffer_type{ buffer, offset, indexType };
}

struct bind_vertex_buffers_type {
	std::vector<type::supplier<buffer::buffer_type>> buffers;
	std::vector<VkDeviceSize> offsets;
};

inline bind_vertex_buffers_type bind_vertex_buffers(uint32_t firstBinding,
	const std::vector<type::supplier<buffer::buffer_type>> &buffers,
	const std::vector<VkDeviceSize> &offsets) {
	return bind_vertex_buffers_type{ buffers, offsets };
}

struct bind_vertex_data_buffers_type {
	std::vector<type::supplier<input_buffer::input_buffer_type>> buffers;
	std::vector<VkDeviceSize> offsets;
};

inline bind_vertex_data_buffers_type bind_vertex_buffers(
	const std::vector<type::supplier<input_buffer::input_buffer_type>> &buffers,
	const std::vector<VkDeviceSize> &offsets) {
	return bind_vertex_data_buffers_type{ buffers, offsets };
}

struct draw {
	uint32_t vertexCount, instanceCount, firstVertex, firstInstance;
};

struct draw_indexed {
	uint32_t indexCount, instanceCount, firstIndex, vertexOffset,
		firstInstance;
};

struct draw_indirect_type {
	type::supplier<buffer::buffer_type> buffer;
	VkDeviceSize offset;
	uint32_t drawCount, stride;
};

inline draw_indirect_type draw_indirect(
	const type::supplier<buffer::buffer_type> &buffer,
	VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
	return draw_indirect_type{ buffer, offset, drawCount, stride };
}

struct draw_indirect_data_type {
	type::supplier<input_buffer::input_buffer_type> buffer;
	VkDeviceSize offset;
	uint32_t drawCount, stride;
};

inline draw_indirect_data_type draw_indirect(
	const type::supplier<input_buffer::input_buffer_type> &buffer,
	VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
	return draw_indirect_data_type{ buffer, offset, drawCount, stride };
}

struct draw_indexed_indirect_type {
	type::supplier<buffer::buffer_type> buffer;
	VkDeviceSize offset;
	uint32_t drawCount, stride;
};

inline draw_indexed_indirect_type draw_indexed_indirect(
	const type::supplier<buffer::buffer_type> &buffer,
	VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
	return draw_indexed_indirect_type{ buffer, offset, drawCount, stride };
}

struct draw_indexed_indirect_data_type {
	type::supplier<input_buffer::input_buffer_type> buffer;
	VkDeviceSize offset;
	uint32_t drawCount, stride;
};

inline draw_indexed_indirect_data_type draw_indexed_indirect_data(
	const type::supplier<input_buffer::input_buffer_type> &buffer,
	VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
	return draw_indexed_indirect_data_type{ buffer, offset, drawCount, stride };
}

struct dispatch {
	uint32_t x, y, z;
};

struct dispatch_indirect_type {
	type::supplier<buffer::buffer_type> buffer;
	VkDeviceSize offset;
};

inline dispatch_indirect_type dispatch_indirect(
	const type::supplier<buffer::buffer_type> &buffer, VkDeviceSize offset) {
	return dispatch_indirect_type{ buffer, offset };
}

struct dispatch_indirect_data_type {
	type::supplier<input_buffer::input_buffer_type> buffer;
	VkDeviceSize offset;
};

inline dispatch_indirect_data_type dispatch_indirect_data(
	const type::supplier<input_buffer::input_buffer_type> &buffer,
	VkDeviceSize offset) {
	return dispatch_indirect_data_type{ buffer, offset };
}

struct copy_buffer_type {
	type::supplier<buffer::buffer_type> srcBuffer, dstBuffer;
	std::vector<VkBufferCopy> regions;
};

inline copy_buffer_type copy_buffer(
	const type::supplier<buffer::buffer_type> &srcBuffer,
	type::supplier<buffer::buffer_type> &&dstBuffer,
	const std::vector<VkBufferCopy> &regions) {
	return copy_buffer_type{ srcBuffer, dstBuffer, regions };
}

struct copy_data_buffer_type {
	type::supplier<input_buffer::input_buffer_type> srcBuffer;
	type::supplier<buffer::buffer_type> dstBuffer;
	std::vector<VkBufferCopy> regions;
};

inline copy_data_buffer_type copy_data_buffer(
	const type::supplier<input_buffer::input_buffer_type> &srcBuffer,
	const type::supplier<buffer::buffer_type> &dstBuffer,
	const std::vector<VkBufferCopy> &regions) {
	return copy_data_buffer_type{ srcBuffer, dstBuffer, regions };
}

struct copy_image {
	type::supplier<image::image_type> srcImage;
	VkImageLayout srcImageLayout;
	type::supplier<image::image_type> dstImage;
	VkImageLayout dstImageLayout;
	std::vector<VkImageCopy> regions;
};

struct blit_image {
	type::supplier<image::image_type> srcImage;
	VkImageLayout srcImageLayout;
	type::supplier<image::image_type> dstImage;
	VkImageLayout dstImageLayout;
	std::vector<VkImageBlit> regions;
	VkFilter filter;
};

struct copy_buffer_to_image_type {
	type::supplier<buffer::buffer_type> srcBuffer;
	type::supplier<image::image_type> dstImage;
	VkImageLayout dstImageLayout;
	std::vector<VkBufferImageCopy> regions;
};

inline copy_buffer_to_image_type copy_buffer_to_image(
	const type::supplier<buffer::buffer_type> &srcBuffer,
	const type::supplier<image::image_type> &dstImage,
	VkImageLayout dstImageLayout,
	const std::vector<VkBufferImageCopy> &regions) {
	return copy_buffer_to_image_type{
		srcBuffer, dstImage, dstImageLayout, regions };
}

struct copy_data_buffer_to_image_type {
	type::supplier<input_buffer::input_buffer_type> srcBuffer;
	type::supplier<image::image_type> dstImage;
	VkImageLayout dstImageLayout;
	std::vector<VkBufferImageCopy> regions;
};

inline copy_data_buffer_to_image_type copy_data_buffer_to_image(
	const type::supplier<input_buffer::input_buffer_type> &srcBuffer,
	const type::supplier<image::image_type> &dstImage,
	VkImageLayout dstImageLayout,
	const std::vector<VkBufferImageCopy> &regions) {
	return copy_data_buffer_to_image_type{
		srcBuffer, dstImage,
		dstImageLayout, regions };
}

struct copy_image_to_buffer {
	type::supplier<image::image_type> srcImage;
	VkImageLayout srcImageLayout;
	type::supplier<buffer::buffer_type> dstBuffer;
	std::vector<VkBufferImageCopy> regions;
};

struct update_buffer {
	type::supplier<buffer::buffer_type> dstBuffer;
	VkDeviceSize dstOffset, dataSize;
	const uint32_t* pData;
};

struct fill_buffer {
	type::supplier<buffer::buffer_type> dstBuffer;
	VkDeviceSize dstOffset, size;
	uint32_t data;
};

struct clear_color_image {
	type::supplier<image::image_type> image;
	VkImageLayout imageLayout;
	VkClearColorValue color;
	std::vector<VkImageSubresourceRange> ranges;
};

struct clear_depth_stencil_image {
	type::supplier<image::image_type> image;
	VkImageLayout imageLayout;
	VkClearDepthStencilValue pDepthStencil;
	std::vector<VkImageSubresourceRange> ranges;
};

struct clear_attachments {
	std::vector<VkClearAttachment> attachments;
	std::vector<VkClearRect> rects;
};

struct resolve_image {
	type::supplier<image::image_type> srcImage;
	VkImageLayout srcImageLayout;
	type::supplier<image::image_type> dstImage;
	VkImageLayout dstImageLayout;
	std::vector<VkImageResolve> regions;
};

struct set_event {
	type::supplier<event::event_type> event;
	VkPipelineStageFlags stageMask;
};

struct reset_event {
	type::supplier<event::event_type> event;
	VkPipelineStageFlags stageMask;
};

struct wait_events {
	std::vector<type::supplier<event::event_type>> events;
	VkPipelineStageFlags srcStageMask, dstStageMask;
	std::vector<VkMemoryBarrier> memoryBarriers;
	std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;
	std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
};

struct memory_barrier {
	VkAccessFlags srcAccessMask, dstAccessMask;
};

struct buffer_memory_barrier_type {
	VkAccessFlags srcAccessMask, dstAccessMask;
	uint32_t srcQueueFamilyIndex, dstQueueFamilyIndex;
	type::supplier<buffer::buffer_type> buffer;
	VkDeviceSize offset, size;
};

inline buffer_memory_barrier_type buffer_memory_barrier(
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
	uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex,
	const type::supplier<buffer::buffer_type> &buffer,
	VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) {
	return buffer_memory_barrier_type{
		srcAccessMask, dstAccessMask,
		srcQueueFamilyIndex, dstQueueFamilyIndex,
		buffer, offset, size
	};
}

inline buffer_memory_barrier_type buffer_memory_barrier(
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
	uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex,
	const type::supplier<input_buffer::input_buffer_type> &buffer,
	VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) {
	return buffer_memory_barrier_type{
		srcAccessMask, dstAccessMask,
		srcQueueFamilyIndex, dstQueueFamilyIndex,
		std::ref(input_buffer::internal::get_buffer(*buffer)), offset, size
	};
}

struct image_memory_barrier {
	VkAccessFlags srcAccessMask;
	VkAccessFlags dstAccessMask;
	VkImageLayout oldLayout;
	VkImageLayout newLayout;
	uint32_t srcQueueFamilyIndex;
	uint32_t dstQueueFamilyIndex;
	type::supplier<image::image_type> image;
	VkImageSubresourceRange subresourceRange;
};

struct pipeline_barrier {
	pipeline_barrier(VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
		const std::vector<memory_barrier> memory_barriers,
		const std::vector<buffer_memory_barrier_type> buffer_memory_barriers,
		const std::vector<image_memory_barrier> image_memory_barriers)
		: srcStageMask(srcStageMask), dstStageMask(dstStageMask),
		dependencyFlags(dependencyFlags), memory_barriers(memory_barriers),
		buffer_memory_barriers(buffer_memory_barriers),
		image_memory_barriers(image_memory_barriers) {}

	VkPipelineStageFlags srcStageMask, dstStageMask;
	VkDependencyFlags dependencyFlags;
	std::vector<memory_barrier> memory_barriers;
	std::vector<buffer_memory_barrier_type> buffer_memory_barriers;
	std::vector<image_memory_barrier> image_memory_barriers;
};

struct begin_query {
	type::supplier<query_pool::query_pool_type> queryPool;
	uint32_t entry;
	VkQueryControlFlags flags;
};

struct end_query {
	type::supplier<query_pool::query_pool_type> queryPool;
	uint32_t entry;
};

struct reset_query_pool {
	type::supplier<query_pool::query_pool_type> queryPool;
	uint32_t firstQuery, queryCount;
};

struct write_timestamp {
	VkPipelineStageFlagBits pipelineStage;
	type::supplier<query_pool::query_pool_type> queryPool;
	uint32_t entry;
};

struct copy_query_pool_results {
	type::supplier<query_pool::query_pool_type> queryPool;
	uint32_t firstQuery, queryCount;
	type::supplier<buffer::buffer_type> dstBuffer;
	VkDeviceSize dstOffset, stride;
	VkQueryResultFlags flags;
};

struct push_constants_type {
	type::supplier<pipeline_layout::pipeline_layout_type> layout;
	VkShaderStageFlags stageFlags;
	uint32_t offset, size;
	const void* pValues;
};

inline push_constants_type push_constants(
	const type::supplier<pipeline_layout::pipeline_layout_type> &layout,
	VkShaderStageFlags stageFlags,
	uint32_t offset, uint32_t size,
	const void* pValues) {
	return push_constants_type{ layout, stageFlags, offset, size, pValues };
}

template<typename... CommandsT>
struct render_pass_type {
	type::supplier<vcc::render_pass::render_pass_type> renderPass;
	type::supplier<framebuffer::framebuffer_type> framebuffer;
	VkRect2D renderArea;
	std::vector<VkClearValue> clearValues;
	VkSubpassContents contents;
	std::tuple<CommandsT...> commands;
};

template<typename... CommandsT>
render_pass_type<CommandsT...> render_pass(
	const type::supplier<render_pass::render_pass_type> &renderPass,
	const type::supplier<framebuffer::framebuffer_type> &framebuffer,
	const VkRect2D &renderArea, const std::vector<VkClearValue> &clearValues,
	VkSubpassContents contents, CommandsT&&...  commands) {
	auto commands_tuple = std::make_tuple(std::forward<CommandsT>(commands)...);
	auto render_pass = render_pass_type<CommandsT...>{
		renderPass, framebuffer, renderArea, clearValues, contents,
		std::move(commands_tuple) };
	return std::move(render_pass);
}

struct next_subpass {
	VkSubpassContents contents;
};

struct execute_commands {
	std::vector<type::supplier<command_buffer::command_buffer_type>> commandBuffers;
};

VCC_LIBRARY VkClearValue clear_color(const VkClearColorValue & color);

VCC_LIBRARY VkClearValue clear_depth_stencil(
	const VkClearDepthStencilValue &depth_stencil);

namespace internal {

struct cmd_args {
	std::reference_wrapper<command_buffer::command_buffer_type> buffer;
	vcc::internal::hook_container_type<queue::queue_type&>
		pre_execute_callbacks;
	vcc::internal::reference_container_type references;
};

VCC_LIBRARY void cmd(cmd_args &, const bind_pipeline &);
VCC_LIBRARY void cmd(cmd_args &, const set_viewport &);
VCC_LIBRARY void cmd(cmd_args &, const set_scissor &);
VCC_LIBRARY void cmd(cmd_args &, const set_line_width &);
VCC_LIBRARY void cmd(cmd_args &, const set_depth_bias &);
VCC_LIBRARY void cmd(cmd_args &, const set_blend_constants &);
VCC_LIBRARY void cmd(cmd_args &, const set_depth_bounds &);
VCC_LIBRARY void cmd(cmd_args &, const set_stencil_compare_mask &);
VCC_LIBRARY void cmd(cmd_args &, const set_stencil_write_mask &);
VCC_LIBRARY void cmd(cmd_args &, const set_stencil_reference &);
VCC_LIBRARY void cmd(cmd_args &, const bind_descriptor_sets &);
VCC_LIBRARY void cmd(cmd_args &, const bind_index_buffer_type &);
VCC_LIBRARY void cmd(cmd_args &, const bind_vertex_buffers_type &);
VCC_LIBRARY void cmd(cmd_args &, const draw &);
VCC_LIBRARY void cmd(cmd_args &, const draw_indexed &);
VCC_LIBRARY void cmd(cmd_args &, const draw_indirect_type &);
VCC_LIBRARY void cmd(cmd_args &, const draw_indexed_indirect_type &);
VCC_LIBRARY void cmd(cmd_args &, const dispatch &);
VCC_LIBRARY void cmd(cmd_args &, const dispatch_indirect_type &);
VCC_LIBRARY void cmd(cmd_args &, const copy_buffer_type &);
VCC_LIBRARY void cmd(cmd_args &, const copy_image &);
VCC_LIBRARY void cmd(cmd_args &, const blit_image &);
VCC_LIBRARY void cmd(cmd_args &, const copy_buffer_to_image_type &);
VCC_LIBRARY void cmd(cmd_args &, const copy_image_to_buffer &);
VCC_LIBRARY void cmd(cmd_args &, const update_buffer &);
VCC_LIBRARY void cmd(cmd_args &, const fill_buffer &);
VCC_LIBRARY void cmd(cmd_args &, const clear_color_image &);
VCC_LIBRARY void cmd(cmd_args &, const clear_depth_stencil_image &);
VCC_LIBRARY void cmd(cmd_args &, const clear_attachments &);
VCC_LIBRARY void cmd(cmd_args &, const resolve_image &);
VCC_LIBRARY void cmd(cmd_args &, const set_event &);
VCC_LIBRARY void cmd(cmd_args &, const reset_event &);
VCC_LIBRARY void cmd(cmd_args &, const wait_events &);
VCC_LIBRARY void cmd(cmd_args &, const pipeline_barrier &);
VCC_LIBRARY void cmd(cmd_args &, const begin_query &);
VCC_LIBRARY void cmd(cmd_args &, const end_query &);
VCC_LIBRARY void cmd(cmd_args &, const reset_query_pool &);
VCC_LIBRARY void cmd(cmd_args &, const write_timestamp &);
VCC_LIBRARY void cmd(cmd_args &, const copy_query_pool_results &);
VCC_LIBRARY void cmd(cmd_args &, const push_constants_type &);
VCC_LIBRARY void cmd(cmd_args &, const next_subpass &);
VCC_LIBRARY void cmd(cmd_args &, const execute_commands &);
VCC_LIBRARY void cmd(cmd_args &, const bind_index_data_buffer_type&);
VCC_LIBRARY void cmd(cmd_args &, const bind_vertex_data_buffers_type&);
VCC_LIBRARY void cmd(cmd_args &, const draw_indirect_data_type&);
VCC_LIBRARY void cmd(cmd_args &, const draw_indexed_indirect_data_type&);
VCC_LIBRARY void cmd(cmd_args &, const dispatch_indirect_data_type&);
VCC_LIBRARY void cmd(cmd_args &, const copy_data_buffer_type&);
VCC_LIBRARY void cmd(cmd_args &, const copy_data_buffer_to_image_type&);

// Need C++14 to do auto argument lambdas.
struct call_cmd_type {
	cmd_args &args;

	template<typename T>
	void operator()(T &value) {
		cmd(args, value);
	};
};

template<typename... CommandsT>
void cmd(cmd_args &args, render_pass_type<CommandsT...> &&render_pass) {
	VkRenderPassBeginInfo info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		NULL };
	info.renderPass = vcc::internal::get_instance(*render_pass.renderPass);
	info.framebuffer = vcc::internal::get_instance(*render_pass.framebuffer);
	info.renderArea = render_pass.renderArea;
	info.clearValueCount = (uint32_t)render_pass.clearValues.size();
	info.pClearValues = render_pass.clearValues.data();
	VKTRACE(vkCmdBeginRenderPass(
		vcc::internal::get_instance(args.buffer.get()), &info,
		render_pass.contents));
	util::tuple_foreach(call_cmd_type{ args }, render_pass.commands);
	VKTRACE(vkCmdEndRenderPass(
		vcc::internal::get_instance(args.buffer.get())));
	args.references.add(render_pass.renderPass);
	args.references.add(render_pass.framebuffer);
}

}  // namespace internal

}  // namespace command

namespace command_buffer {

template<typename... CommandsT>
void compile(command_buffer_type &command_buffer,
		VkCommandBufferUsageFlags flags,
		render_pass::render_pass_type &render_pass,
		uint32_t subpass,
		framebuffer::framebuffer_type &framebuffer,
		VkBool32 occlusionQueryEnable, VkQueryControlFlags queryFlags,
		VkQueryPipelineStatisticFlags pipelineStatistics,
		CommandsT&&... commands) {
	begin_type begin_instance(begin(std::ref(command_buffer), flags,
		std::ref(render_pass), subpass, std::ref(framebuffer), occlusionQueryEnable,
		queryFlags, pipelineStatistics));
	command::internal::cmd_args args = { command_buffer };
	args.references.add(render_pass, framebuffer);
	// int array guarantees order of execution with older GCC compilers.
	const int dummy[] = { (command::internal::cmd(args, std::forward<CommandsT>(commands)), 0)... };
	command_buffer.references = std::move(args.references);
	command_buffer.pre_execute_hook = std::move(args.pre_execute_callbacks);
}

template<typename... CommandsT>
void compile(command_buffer_type &command_buffer,
		VkCommandBufferUsageFlags flags,
		VkBool32 occlusionQueryEnable, VkQueryControlFlags queryFlags,
		VkQueryPipelineStatisticFlags pipelineStatistics,
		CommandsT&&... commands) {
	begin_type begin_instance(begin(std::ref(command_buffer), flags,
		occlusionQueryEnable, queryFlags, pipelineStatistics));
	command::internal::cmd_args args = { command_buffer };
	// int array guarantees order of execution with older GCC compilers.
	const int dummy[] = { (command::internal::cmd(args, std::forward<CommandsT>(commands)), 0)... };
	command_buffer.references = std::move(args.references);
	command_buffer.pre_execute_hook = std::move(args.pre_execute_callbacks);
}

}  // namespace command_buffer
}  // namespace vcc

#endif  // _VCC_COMMAND_H_
