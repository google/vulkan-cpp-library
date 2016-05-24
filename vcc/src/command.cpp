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
#include <vcc/command.h>

namespace vcc {
namespace command {

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
	VKTRACE(vkCmdBindPipeline(vcc::internal::get_instance(args.buffer.get()),
		bp.pipelineBindPoint, vcc::internal::get_instance(*bp.pipeline)));
	args.references.add(bp.pipeline);
}

void cmd(cmd_args &args, const set_viewport &sv) {
	VKTRACE(vkCmdSetViewport(vcc::internal::get_instance(args.buffer.get()),
		sv.first_viewport, (uint32_t)sv.viewports.size(),
		sv.viewports.data()));
}

void cmd(cmd_args &args, const set_scissor &ss) {
	VKTRACE(vkCmdSetScissor(vcc::internal::get_instance(args.buffer.get()),
		ss.first_scissor, (uint32_t)ss.scissors.size(), ss.scissors.data()));
}

void cmd(cmd_args &args, const set_line_width &slw) {
	VKTRACE(vkCmdSetLineWidth(vcc::internal::get_instance(args.buffer.get()),
		slw.lineWidth));
}

void cmd(cmd_args &args, const set_depth_bias &sdb) {
	VKTRACE(vkCmdSetDepthBias(vcc::internal::get_instance(args.buffer.get()),
		sdb.depthBiasConstantFactor, sdb.depthBiasClamp, sdb.depthBiasSlopeFactor));
}

void cmd(cmd_args &args, const set_blend_constants &sbc) {
	VKTRACE(vkCmdSetBlendConstants(vcc::internal::get_instance(args.buffer.get()),
		sbc.blendConstants.data()));
}

void cmd(cmd_args &args, const set_depth_bounds &sdb) {
	VKTRACE(vkCmdSetDepthBounds(vcc::internal::get_instance(args.buffer.get()),
		sdb.minDepthBounds, sdb.maxDepthBounds));
}

void cmd(cmd_args &args, const set_stencil_compare_mask &sscm) {
	VKTRACE(vkCmdSetStencilCompareMask(
		vcc::internal::get_instance(args.buffer.get()),
		sscm.faceMask, sscm.compareMask));
}

void cmd(cmd_args &args, const set_stencil_write_mask &sswm) {
	VKTRACE(vkCmdSetStencilWriteMask(
		vcc::internal::get_instance(args.buffer.get()),
		sswm.faceMask, sswm.writeMask));
}

void cmd(cmd_args &args, const set_stencil_reference &ssr) {
	VKTRACE(vkCmdSetStencilReference(
		vcc::internal::get_instance(args.buffer.get()),
		ssr.faceMask, ssr.reference));
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
		args.pre_execute_callbacks.add([descriptor_set](queue::queue_type &queue) {
			descriptor_set->pre_execute_callbacks(queue);
		});
	}
	VKTRACE(vkCmdBindDescriptorSets(
		vcc::internal::get_instance(args.buffer.get()), bds.pipelineBindPoint,
		vcc::internal::get_instance(*layout), bds.firstSet,
		(uint32_t)bds.descriptor_sets.size(), descriptor_sets.data(),
		(uint32_t)bds.dynamic_offsets.size(), bds.dynamic_offsets.data()));
}

void cmd(cmd_args &args, const bind_index_buffer_type &bib) {
	VKTRACE(vkCmdBindIndexBuffer(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*bib.buffer), bib.offset, bib.indexType));
	args.references.add(bib.buffer);
}

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
	VKTRACE(vkCmdBindVertexBuffers(
		vcc::internal::get_instance(args.buffer.get()), 0,
		(uint32_t)bvb.buffers.size(), buffers.data(), offsets.data()));
}

void cmd(cmd_args &args, const draw &d) {
	VKTRACE(vkCmdDraw(vcc::internal::get_instance(args.buffer.get()),
		d.vertexCount, d.instanceCount, d.firstVertex, d.firstInstance));
}

void cmd(cmd_args &args, const draw_indexed &di) {
	VKTRACE(vkCmdDrawIndexed(vcc::internal::get_instance(args.buffer.get()),
		di.indexCount, di.instanceCount, di.firstIndex, di.vertexOffset,
		di.firstInstance));
}

void cmd(cmd_args &args, const draw_indirect_type &di) {
	VKTRACE(vkCmdDrawIndirect(vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*di.buffer), di.offset, di.drawCount,
		di.stride));
}

void cmd(cmd_args &args, const draw_indexed_indirect_type &dii) {
	VKTRACE(vkCmdDrawIndexedIndirect(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*dii.buffer), dii.offset, dii.drawCount,
		dii.stride));
}

void cmd(cmd_args &args, const dispatch &d) {
	VKTRACE(vkCmdDispatch(vcc::internal::get_instance(args.buffer.get()),
		d.x, d.y, d.z));
}

void cmd(cmd_args &args, const dispatch_indirect_type &di) {
	VKTRACE(vkCmdDispatchIndirect(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*di.buffer), di.offset));
	args.references.add(di.buffer);
}

void cmd(cmd_args &args, const copy_buffer_type &cb) {
	VKTRACE(vkCmdCopyBuffer(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*cb.srcBuffer),
		vcc::internal::get_instance(*cb.dstBuffer),
		(uint32_t)cb.regions.size(), cb.regions.data()));
	args.references.add(cb.srcBuffer);
	args.references.add(cb.dstBuffer);
}

void cmd(cmd_args &args, const copy_image &ci) {
	VKTRACE(vkCmdCopyImage(vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*ci.srcImage), ci.srcImageLayout,
		vcc::internal::get_instance(*ci.dstImage), ci.dstImageLayout,
		(uint32_t)ci.regions.size(), ci.regions.data()));
	args.references.add(ci.srcImage);
	args.references.add(ci.dstImage);
}

void cmd(cmd_args &args, const blit_image &bi) {
	VKTRACE(vkCmdBlitImage(vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*bi.srcImage), bi.srcImageLayout,
		vcc::internal::get_instance(*bi.dstImage), bi.dstImageLayout,
		(uint32_t)bi.regions.size(), bi.regions.data(), bi.filter));
	args.references.add(bi.srcImage);
	args.references.add(bi.dstImage);
}

void cmd(cmd_args &args, const copy_buffer_to_image_type &bti) {
	VKTRACE(vkCmdCopyBufferToImage(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*bti.srcBuffer),
		vcc::internal::get_instance(*bti.dstImage), bti.dstImageLayout,
		(uint32_t)bti.regions.size(), bti.regions.data()));
	args.references.add(bti.srcBuffer);
	args.references.add(bti.dstImage);
}

void cmd(cmd_args &args, const copy_image_to_buffer &cib) {
	VKTRACE(vkCmdCopyImageToBuffer(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*cib.srcImage), cib.srcImageLayout,
		vcc::internal::get_instance(*cib.dstBuffer),
		(uint32_t)cib.regions.size(), cib.regions.data()));
	args.references.add(cib.srcImage);
	args.references.add(cib.dstBuffer);
}

void cmd(cmd_args &args, const update_buffer &ub) {
	VKTRACE(vkCmdUpdateBuffer(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*ub.dstBuffer), ub.dstOffset, ub.dataSize,
		ub.pData));
	args.references.add(ub.dstBuffer);
}

void cmd(cmd_args &args, const fill_buffer &fb) {
	VKTRACE(vkCmdFillBuffer(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*fb.dstBuffer), fb.dstOffset, fb.size,
		fb.data));
	args.references.add(fb.dstBuffer);
}

void cmd(cmd_args &args, const clear_color_image &cci) {
	VKTRACE(vkCmdClearColorImage(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*cci.image), cci.imageLayout, &cci.color,
		(uint32_t)cci.ranges.size(), cci.ranges.data()));
	args.references.add(cci.image);
}

void cmd(cmd_args &args, const clear_depth_stencil_image &cdsi) {
	VKTRACE(vkCmdClearDepthStencilImage(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*cdsi.image), cdsi.imageLayout,
		&cdsi.pDepthStencil, (uint32_t)cdsi.ranges.size(),
		cdsi.ranges.data()));
	args.references.add(cdsi.image);
}

void cmd(cmd_args &args, const clear_attachments &ca) {
	VKTRACE(vkCmdClearAttachments(
		vcc::internal::get_instance(args.buffer.get()),
		(uint32_t)ca.attachments.size(), ca.attachments.data(),
		(uint32_t)ca.rects.size(), ca.rects.data()));
}

void cmd(cmd_args &args, const resolve_image &ri) {
	VKTRACE(vkCmdResolveImage(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*ri.srcImage), ri.srcImageLayout,
		vcc::internal::get_instance(*ri.dstImage), ri.dstImageLayout,
		(uint32_t)ri.regions.size(), ri.regions.data()));
	args.references.add(ri.srcImage);
	args.references.add(ri.dstImage);
}

void cmd(cmd_args &args, const set_event &se) {
	VKTRACE(vkCmdSetEvent(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*se.event), se.stageMask));
	args.references.add(se.event);
}

void cmd(cmd_args &args, const reset_event &re) {
	VKTRACE(vkCmdResetEvent(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*re.event), re.stageMask));
	args.references.add(re.event);
}

void cmd(cmd_args &args, const wait_events &we) {
	std::vector<VkEvent> events;
	events.reserve(we.events.size());
	for (const type::supplier<event::event_type> &event : we.events) {
		events.push_back(vcc::internal::get_instance(*event));
		args.references.add(event);
	}
	VKTRACE(vkCmdWaitEvents(vcc::internal::get_instance(args.buffer.get()),
		(uint32_t)we.events.size(), events.data(), we.srcStageMask,
		we.dstStageMask, (uint32_t)we.memoryBarriers.size(),
		we.memoryBarriers.data(), (uint32_t)we.bufferMemoryBarriers.size(),
		we.bufferMemoryBarriers.data(),
		(uint32_t)we.imageMemoryBarriers.size(),
		we.imageMemoryBarriers.data()));
}

void cmd(cmd_args &args, const pipeline_barrier &pb) {
	std::vector<VkMemoryBarrier> memory_barriers;
	memory_barriers.reserve(pb.memory_barriers.size());
	for (const memory_barrier &barrier : pb.memory_barriers) {
		memory_barriers.push_back(VkMemoryBarrier{
			VK_STRUCTURE_TYPE_MEMORY_BARRIER, NULL, barrier.srcAccessMask,
			barrier.dstAccessMask });
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
	VKTRACE(vkCmdPipelineBarrier(
		vcc::internal::get_instance(args.buffer.get()), pb.srcStageMask,
		pb.dstStageMask, pb.dependencyFlags, (uint32_t)memory_barriers.size(),
		memory_barriers.data(), (uint32_t)buffer_memory_barriers.size(),
		buffer_memory_barriers.data(), (uint32_t)image_memory_barriers.size(),
		image_memory_barriers.data()));
}

void cmd(cmd_args &args, const begin_query &bq) {
	VKTRACE(vkCmdBeginQuery(vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*bq.queryPool), bq.entry, bq.flags));
	args.references.add(bq.queryPool);
}

void cmd(cmd_args &args, const end_query &eq) {
	VKTRACE(vkCmdEndQuery(vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*eq.queryPool), eq.entry));
	args.references.add(eq.queryPool);
}

void cmd(cmd_args &args, const reset_query_pool &rqp) {
	VKTRACE(vkCmdResetQueryPool(vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*rqp.queryPool), rqp.firstQuery,
		rqp.queryCount));
	args.references.add(rqp.queryPool);
}

void cmd(cmd_args &args, const write_timestamp &wt) {
	VKTRACE(vkCmdWriteTimestamp(vcc::internal::get_instance(args.buffer.get()),
		wt.pipelineStage, vcc::internal::get_instance(*wt.queryPool),
		wt.entry));
	args.references.add(wt.queryPool);
}

void cmd(cmd_args &args, const copy_query_pool_results &cqpr) {
	VKTRACE(vkCmdCopyQueryPoolResults(
		vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*cqpr.queryPool), cqpr.firstQuery,
		cqpr.queryCount, vcc::internal::get_instance(*cqpr.dstBuffer),
		cqpr.dstOffset, cqpr.stride, cqpr.flags));
	args.references.add(cqpr.queryPool);
	args.references.add(cqpr.dstBuffer);
}

void cmd(cmd_args &args, const push_constants_type &pc) {
	VKTRACE(vkCmdPushConstants(vcc::internal::get_instance(args.buffer.get()),
		vcc::internal::get_instance(*pc.layout), pc.stageFlags, pc.offset,
		pc.size, pc.pValues));
	args.references.add(pc.layout);
}

void cmd(cmd_args &args, const next_subpass &ns) {
	VKTRACE(vkCmdNextSubpass(vcc::internal::get_instance(args.buffer.get()),
		ns.contents));
}

void cmd(cmd_args &args, const execute_commands &ec) {
	std::vector<VkCommandBuffer> command_buffers;
	command_buffers.reserve(ec.commandBuffers.size());
	for (const type::supplier<command_buffer::command_buffer_type> &command : ec.commandBuffers) {
		command_buffers.push_back(vcc::internal::get_instance(*command));
		args.references.add(command);
	}
	VKTRACE(vkCmdExecuteCommands(
		vcc::internal::get_instance(args.buffer.get()),
		(uint32_t)command_buffers.size(), command_buffers.data()));
}

void cmd(cmd_args &args, const bind_index_data_buffer_type&bidb) {
	const type::supplier<data::buffer_type> &buffer(bidb.buffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer); });
	cmd(args, bind_index_buffer_type{ std::ref(data::internal::get_buffer(*buffer)), bidb.offset, bidb.indexType });
}

void cmd(cmd_args &args, const bind_vertex_data_buffers_type&bvdb) {
	std::vector<type::supplier<buffer::buffer_type>> buffers;
	buffers.reserve(bvdb.buffers.size());
	for (const type::supplier<data::buffer_type> &buffer : bvdb.buffers) {
		args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer); });
		buffers.push_back(std::ref(data::internal::get_buffer(*buffer)));
	}
	cmd(args, bind_vertex_buffers_type{ std::move(buffers), bvdb.offsets });
}

void cmd(cmd_args &args, const draw_indirect_data_type&did) {
	const type::supplier<data::buffer_type> &buffer(did.buffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer); });
	cmd(args, draw_indirect_type{ std::ref(data::internal::get_buffer(*buffer)), did.offset, did.drawCount, did.stride });
}

void cmd(cmd_args &args, const draw_indexed_indirect_data_type&diid) {
	const type::supplier<data::buffer_type> &buffer(diid.buffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer); });
	cmd(args, draw_indexed_indirect_type{ std::ref(data::internal::get_buffer(*buffer)), diid.offset, diid.drawCount, diid.stride });
}

void cmd(cmd_args &args, const dispatch_indirect_data_type&did) {
	const type::supplier<data::buffer_type> &buffer(did.buffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer); });
	cmd(args, dispatch_indirect_type{ std::ref(data::internal::get_buffer(*buffer)), did.offset });
}

void cmd(cmd_args &args, const copy_data_buffer_type&cdb) {
	const type::supplier<data::buffer_type> &buffer(cdb.srcBuffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer); });
	cmd(args, copy_buffer_type{ std::ref(data::internal::get_buffer(*buffer)), cdb.dstBuffer, cdb.regions });
}

void cmd(cmd_args &args, const copy_data_buffer_to_image_type&cdbti) {
	const type::supplier<data::buffer_type> &buffer(cdbti.srcBuffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer); });
	cmd(args, copy_buffer_to_image_type{ std::ref(data::internal::get_buffer(*buffer)), cdbti.dstImage, cdbti.dstImageLayout, cdbti.regions });
}

}  // namespace internal

}  // namespace command
}  // namespace vcc
