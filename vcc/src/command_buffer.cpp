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
#include <vcc/command_buffer.h>

namespace vcc {
namespace command_buffer {

VkClearValue clear_color(const VkClearColorValue & color) {
	VkClearValue value;
	value.color = color;
	return value;
}

VkClearValue clear_depth_stencil(const VkClearDepthStencilValue &depth_stencil) {
	VkClearValue value;
	value.depthStencil = depth_stencil;
	return value;
}

namespace internal {

void cmd(cmd_args &args, const bind_pipeline &bp) {
	VKTRACE(vkCmdBindPipeline(args.buffer, bp.pipelineBindPoint, vcc::internal::get_instance(*bp.pipeline)));
	args.references.add(bp.pipeline);
}

void cmd(cmd_args &args, const set_viewport &sv) {
	VKTRACE(vkCmdSetViewport(args.buffer, sv.first_viewport, (uint32_t) sv.viewports.size(), sv.viewports.data()));
}

void cmd(cmd_args &args, const set_scissor &ss) {
	VKTRACE(vkCmdSetScissor(args.buffer, ss.first_scissor, (uint32_t)ss.scissors.size(), ss.scissors.data()));
}

void cmd(cmd_args &args, const set_line_width &slw) {
	VKTRACE(vkCmdSetLineWidth(args.buffer, slw.lineWidth));
}

void cmd(cmd_args &args, const set_depth_bias &sdb) {
	VKTRACE(vkCmdSetDepthBias(args.buffer, sdb.depthBiasConstantFactor, sdb.depthBiasClamp, sdb.depthBiasSlopeFactor));
}

void cmd(cmd_args &args, const set_blend_constants &sbc) {
	VKTRACE(vkCmdSetBlendConstants(args.buffer, sbc.blendConstants.data()));
}

void cmd(cmd_args &args, const set_depth_bounds &sdb) {
	VKTRACE(vkCmdSetDepthBounds(args.buffer, sdb.minDepthBounds, sdb.maxDepthBounds));
}

void cmd(cmd_args &args, const set_stencil_compare_mask &sscm) {
	VKTRACE(vkCmdSetStencilCompareMask(args.buffer, sscm.faceMask, sscm.compareMask));
}

void cmd(cmd_args &args, const set_stencil_write_mask &sswm) {
	VKTRACE(vkCmdSetStencilWriteMask(args.buffer, sswm.faceMask, sswm.writeMask));
}

void cmd(cmd_args &args, const set_stencil_reference &ssr) {
	VKTRACE(vkCmdSetStencilReference(args.buffer, ssr.faceMask, ssr.reference));
}

void cmd(cmd_args &args, const bind_descriptor_sets &bds) {
	std::vector<VkDescriptorSet> descriptor_sets;
	descriptor_sets.reserve(bds.descriptor_sets.size());
	type::supplier<pipeline_layout::pipeline_layout_type> layout(bds.layout);
	args.references.add(layout);
	args.pre_execute_callbacks.add([layout](queue::queue_type &queue) {
		pipeline_layout::internal::get_pre_execute_callbacks(*layout)(queue);
	});
	for (const type::supplier<descriptor_set::descriptor_set_type> &descriptor_set : bds.descriptor_sets) {
		descriptor_sets.push_back(vcc::internal::get_instance(*descriptor_set));
		args.references.add(descriptor_set);
		// TODO(gardell): supplier::operator() should handle this.
		args.pre_execute_callbacks.add([descriptor_set](queue::queue_type &queue) {
			descriptor_set->pre_execute_callbacks(queue);
		});
	}
	VKTRACE(vkCmdBindDescriptorSets(args.buffer, bds.pipelineBindPoint,
		vcc::internal::get_instance(*layout), bds.firstSet,
		(uint32_t)bds.descriptor_sets.size(), descriptor_sets.data(),
		(uint32_t)bds.dynamic_offsets.size(), bds.dynamic_offsets.data()));
}

void cmd(cmd_args &args, const bind_index_buffer_type &bib) {
	VKTRACE(vkCmdBindIndexBuffer(args.buffer,
		vcc::internal::get_instance(*bib.buffer), bib.offset, bib.indexType));
	args.references.add(bib.buffer);
}

// TODO(gardell): bvb shouldn't be constant but rvalue and its content consumed!
void cmd(cmd_args &args, const bind_vertex_buffers_type &bvb) {
	std::vector<VkBuffer> buffers;
	buffers.reserve(bvb.buffers.size());
	std::vector<VkDeviceSize> offsets;
	offsets.reserve(bvb.buffers.size());
	//for (const type::supplier<buffer::buffer_type> &buffer : bvb.buffers) {
	for (std::size_t i = 0; i < bvb.buffers.size(); ++i) {
		const type::supplier<buffer::buffer_type> &buffer(bvb.buffers[i]);
		buffers.push_back(vcc::internal::get_instance(*buffer));
		args.references.add(buffer);
		// TODO(gardell): The bottom seem more correct when mapping multiple objects to the same target,
		// but can't find anything in the spec.
		offsets.push_back(bvb.offsets[i]);
		//offsets.push_back(bvb.buffers[i]->offset + bvb.offsets[i]);
	}
	VKTRACE(vkCmdBindVertexBuffers(args.buffer, 0, (uint32_t)bvb.buffers.size(), buffers.data(), offsets.data()));
}

void cmd(cmd_args &args, const draw &d) {
	VKTRACE(vkCmdDraw(args.buffer, d.vertexCount, d.instanceCount, d.firstVertex, d.firstInstance));
}

void cmd(cmd_args &args, const draw_indexed &di) {
	VKTRACE(vkCmdDrawIndexed(args.buffer, di.indexCount, di.instanceCount, di.firstIndex, di.vertexOffset, di.firstInstance));
}

void cmd(cmd_args &args, const draw_indirect_type &di) {
	VKTRACE(vkCmdDrawIndirect(args.buffer, vcc::internal::get_instance(*di.buffer), di.offset, di.drawCount, di.stride));
}

void cmd(cmd_args &args, const draw_indexed_indirect_type &dii) {
	VKTRACE(vkCmdDrawIndexedIndirect(args.buffer, vcc::internal::get_instance(*dii.buffer), dii.offset, dii.drawCount, dii.stride));
}

void cmd(cmd_args &args, const dispatch &d) {
	VKTRACE(vkCmdDispatch(args.buffer, d.x, d.y, d.z));
}

void cmd(cmd_args &args, const dispatch_indirect_type &di) {
	VKTRACE(vkCmdDispatchIndirect(args.buffer, vcc::internal::get_instance(*di.buffer), di.offset));
	args.references.add(di.buffer);
}

void cmd(cmd_args &args, const copy_buffer_type &cb) {
	VKTRACE(vkCmdCopyBuffer(args.buffer, vcc::internal::get_instance(*cb.srcBuffer),
		vcc::internal::get_instance(*cb.dstBuffer), (uint32_t)cb.regions.size(),
		cb.regions.data()));
	args.references.add(cb.srcBuffer);
	args.references.add(cb.dstBuffer);
}

void cmd(cmd_args &args, const copy_image &ci) {
	VKTRACE(vkCmdCopyImage(args.buffer, vcc::internal::get_instance(*ci.srcImage),
		ci.srcImageLayout, vcc::internal::get_instance(*ci.dstImage),
		ci.dstImageLayout, (uint32_t)ci.regions.size(), ci.regions.data()));
	args.references.add(ci.srcImage);
	args.references.add(ci.dstImage);
}

void cmd(cmd_args &args, const blit_image &bi) {
	VKTRACE(vkCmdBlitImage(args.buffer, vcc::internal::get_instance(*bi.srcImage), bi.srcImageLayout, vcc::internal::get_instance(*bi.dstImage), bi.dstImageLayout, (uint32_t)bi.regions.size(), bi.regions.data(), bi.filter));
	args.references.add(bi.srcImage);
	args.references.add(bi.dstImage);
}

void cmd(cmd_args &args, const copy_buffer_to_image_type &bti) {
	VKTRACE(vkCmdCopyBufferToImage(args.buffer, vcc::internal::get_instance(*bti.srcBuffer), vcc::internal::get_instance(*bti.dstImage), bti.dstImageLayout, (uint32_t)bti.regions.size(), bti.regions.data()));
	args.references.add(bti.srcBuffer);
	args.references.add(bti.dstImage);
}

void cmd(cmd_args &args, const copy_image_to_buffer &cib) {
	VKTRACE(vkCmdCopyImageToBuffer(args.buffer, vcc::internal::get_instance(*cib.srcImage), cib.srcImageLayout, vcc::internal::get_instance(*cib.dstBuffer), (uint32_t)cib.regions.size(), cib.regions.data()));
	args.references.add(cib.srcImage);
	args.references.add(cib.dstBuffer);
}

void cmd(cmd_args &args, const update_buffer &ub) {
	VKTRACE(vkCmdUpdateBuffer(args.buffer, vcc::internal::get_instance(*ub.dstBuffer), ub.dstOffset, ub.dataSize, ub.pData));
	args.references.add(ub.dstBuffer);
}

void cmd(cmd_args &args, const fill_buffer &fb) {
	VKTRACE(vkCmdFillBuffer(args.buffer, vcc::internal::get_instance(*fb.dstBuffer), fb.dstOffset, fb.size, fb.data));
	args.references.add(fb.dstBuffer);
}

void cmd(cmd_args &args, const clear_color_image &cci) {
	VKTRACE(vkCmdClearColorImage(args.buffer, vcc::internal::get_instance(*cci.image), cci.imageLayout, &cci.color, (uint32_t)cci.ranges.size(), cci.ranges.data()));
	args.references.add(cci.image);
}

void cmd(cmd_args &args, const clear_depth_stencil_image &cdsi) {
	VKTRACE(vkCmdClearDepthStencilImage(args.buffer, vcc::internal::get_instance(*cdsi.image), cdsi.imageLayout, &cdsi.pDepthStencil, (uint32_t)cdsi.ranges.size(), cdsi.ranges.data()));
	args.references.add(cdsi.image);
}

void cmd(cmd_args &args, const clear_attachments &ca) {
	VKTRACE(vkCmdClearAttachments(args.buffer, (uint32_t)ca.attachments.size(), ca.attachments.data(), (uint32_t)ca.rects.size(), ca.rects.data()));
}

void cmd(cmd_args &args, const resolve_image &ri) {
	VKTRACE(vkCmdResolveImage(args.buffer, vcc::internal::get_instance(*ri.srcImage), ri.srcImageLayout, vcc::internal::get_instance(*ri.dstImage), ri.dstImageLayout, (uint32_t)ri.regions.size(), ri.regions.data()));
	args.references.add(ri.srcImage);
	args.references.add(ri.dstImage);
}

void cmd(cmd_args &args, const set_event &se) {
	VKTRACE(vkCmdSetEvent(args.buffer, vcc::internal::get_instance(*se.event), se.stageMask));
	args.references.add(se.event);
}

void cmd(cmd_args &args, const reset_event &re) {
	VKTRACE(vkCmdResetEvent(args.buffer, vcc::internal::get_instance(*re.event), re.stageMask));
	args.references.add(re.event);
}

void cmd(cmd_args &args, const wait_events &we) {
	std::vector<VkEvent> events;
	events.reserve(we.events.size());
	for (const type::supplier<event::event_type> &event : we.events) {
		events.push_back(vcc::internal::get_instance(*event));
		args.references.add(event);
	}
	VKTRACE(vkCmdWaitEvents(args.buffer, (uint32_t) we.events.size(),
		events.data(), we.srcStageMask, we.dstStageMask,
		(uint32_t)we.memoryBarriers.size(), we.memoryBarriers.data(),
		(uint32_t)we.bufferMemoryBarriers.size(),
		we.bufferMemoryBarriers.data(),
		(uint32_t) we.imageMemoryBarriers.size(),
		we.imageMemoryBarriers.data()));
}

void cmd(cmd_args &args, const pipeline_barrier &pb) {
	std::vector<VkMemoryBarrier> memory_barriers;
	memory_barriers.reserve(pb.memory_barriers.size());
	for (const memory_barrier &barrier : pb.memory_barriers) {
		memory_barriers.push_back(VkMemoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER, NULL, barrier.srcAccessMask, barrier.dstAccessMask});
	}
	std::vector<VkBufferMemoryBarrier> buffer_memory_barriers;
	for (const buffer_memory_barrier_type &barrier : pb.buffer_memory_barriers) {
		buffer_memory_barriers.push_back(VkBufferMemoryBarrier{
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, NULL,
			barrier.srcAccessMask, barrier.dstAccessMask,
			barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex,
			vcc::internal::get_instance(*barrier.buffer), barrier.offset,
			barrier.size });
	}
	buffer_memory_barriers.reserve(pb.buffer_memory_barriers.size());
	std::vector<VkImageMemoryBarrier> image_memory_barriers;
	image_memory_barriers.reserve(pb.image_memory_barriers.size());
	for (const image_memory_barrier &barrier : pb.image_memory_barriers) {
		image_memory_barriers.push_back(VkImageMemoryBarrier{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL,
			barrier.srcAccessMask, barrier.dstAccessMask, barrier.oldLayout,
			barrier.newLayout, barrier.srcQueueFamilyIndex,
			barrier.dstQueueFamilyIndex,
			vcc::internal::get_instance(*barrier.image),
			barrier.subresourceRange });
	}
	VKTRACE(vkCmdPipelineBarrier(args.buffer, pb.srcStageMask, pb.dstStageMask, pb.dependencyFlags,
		(uint32_t) memory_barriers.size(), memory_barriers.data(),
		(uint32_t) buffer_memory_barriers.size(), buffer_memory_barriers.data(),
		(uint32_t) image_memory_barriers.size(), image_memory_barriers.data()));
}

void cmd(cmd_args &args, const begin_query &bq) {
	VKTRACE(vkCmdBeginQuery(args.buffer, vcc::internal::get_instance(*bq.queryPool), bq.entry, bq.flags));
	args.references.add(bq.queryPool);
}

void cmd(cmd_args &args, const end_query &eq) {
	VKTRACE(vkCmdEndQuery(args.buffer, vcc::internal::get_instance(*eq.queryPool), eq.entry));
	args.references.add(eq.queryPool);
}

void cmd(cmd_args &args, const reset_query_pool &rqp) {
	VKTRACE(vkCmdResetQueryPool(args.buffer, vcc::internal::get_instance(*rqp.queryPool), rqp.firstQuery, rqp.queryCount));
	args.references.add(rqp.queryPool);
}

void cmd(cmd_args &args, const write_timestamp &wt) {
	VKTRACE(vkCmdWriteTimestamp(args.buffer, wt.pipelineStage, vcc::internal::get_instance(*wt.queryPool), wt.entry));
	args.references.add(wt.queryPool);
}

void cmd(cmd_args &args, const copy_query_pool_results &cqpr) {
	VKTRACE(vkCmdCopyQueryPoolResults(args.buffer,
		vcc::internal::get_instance(*cqpr.queryPool), cqpr.firstQuery,
		cqpr.queryCount, vcc::internal::get_instance(*cqpr.dstBuffer),
		cqpr.dstOffset, cqpr.stride, cqpr.flags));
	args.references.add(cqpr.queryPool);
	args.references.add(cqpr.dstBuffer);
}

void cmd(cmd_args &args, const push_constants_type &pc) {
	VKTRACE(vkCmdPushConstants(args.buffer, vcc::internal::get_instance(*pc.layout), pc.stageFlags, pc.offset, pc.size, pc.pValues));
	args.references.add(pc.layout);
}

void cmd(cmd_args &args, const next_subpass &ns) {
	VKTRACE(vkCmdNextSubpass(args.buffer, ns.contents));
}

void cmd(cmd_args &args, const execute_commands &ec) {
	std::vector<VkCommandBuffer> command_buffers;
	command_buffers.reserve(ec.commandBuffers.size());
	for (const type::supplier<command_buffer_type> &command : ec.commandBuffers) {
		command_buffers.push_back(vcc::internal::get_instance(*command));
		args.references.add(command);
	}
	VKTRACE(vkCmdExecuteCommands(args.buffer, (uint32_t)command_buffers.size(), command_buffers.data()));
}

}  // namespace internal

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


