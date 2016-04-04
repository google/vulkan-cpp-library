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
#include <vcc/command_buffer.h>
#include <vcc/data/buffer.h>
#include <vcc/memory.h>
#include <vcc/queue.h>

namespace vcc {
namespace data {

void flush(buffer_type &buffer) {
	if (type::dirty(buffer.serialize)) {
		std::unique_lock<std::mutex> lock(buffer.mutex);
		if (type::dirty(buffer.serialize)) {
			const memory::map_type map(memory::map(
				vcc::internal::get_memory(buffer.buffer),
				vcc::internal::get_offset(buffer.buffer),
				type::size(buffer.serialize)));
			type::flush(buffer.serialize,
				(uint8_t *)map.data + vcc::internal::get_offset(buffer.buffer));
		}
	}
}

void flush(queue::queue_type &queue, buffer_type &buffer) {
	flush(buffer);
	command_pool::command_pool_type command_pool(command_pool::create(
		vcc::internal::get_parent(queue), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, queue::get_family_index(queue)));
	command_buffer::command_buffer_type cmd(std::move(
		command_buffer::allocate(vcc::internal::get_parent(queue),
			std::ref(command_pool), VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1)
		.front()));

	command_buffer::compile(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		VK_FALSE, 0, 0,
		command_buffer::pipeline_barrier{ VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
			{},
			{
				command_buffer::buffer_memory_barrier_type{
					VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
					std::ref(buffer.buffer)}
			},
			{}
		});
	// Must block until our command finish executing.
	fence::fence_type fence(vcc::fence::create(vcc::internal::get_parent(queue)));
	queue::submit(queue, {}, { std::move(cmd) }, {}, fence);
	fence::wait(*vcc::internal::get_parent(queue), { std::ref(fence) }, true,
		std::chrono::nanoseconds::max());
}

}  // namespace data

namespace command_buffer {

namespace internal {

void cmd(cmd_args &args, const bind_index_data_buffer_type&bidb) {
	const type::supplier<data::buffer_type> &buffer(bidb.buffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer);});
	cmd(args, bind_index_buffer_type{std::ref(data::internal::get_buffer(*buffer)), bidb.offset, bidb.indexType});
}

void cmd(cmd_args &args, const bind_vertex_data_buffers_type&bvdb) {
	std::vector<type::supplier<buffer::buffer_type>> buffers;
	buffers.reserve(bvdb.buffers.size());
	for (const type::supplier<data::buffer_type> &buffer : bvdb.buffers) {
		args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer);});
		buffers.push_back(std::ref(data::internal::get_buffer(*buffer)));
	}
	cmd(args, bind_vertex_buffers_type{std::move(buffers), bvdb.offsets});
}

void cmd(cmd_args &args, const draw_indirect_data_type&did) {
	const type::supplier<data::buffer_type> &buffer(did.buffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer);});
	cmd(args, draw_indirect_type{std::ref(data::internal::get_buffer(*buffer)), did.offset, did.drawCount, did.stride});
}

void cmd(cmd_args &args, const draw_indexed_indirect_data_type&diid) {
	const type::supplier<data::buffer_type> &buffer(diid.buffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer);});
	cmd(args, draw_indexed_indirect_type{std::ref(data::internal::get_buffer(*buffer)), diid.offset, diid.drawCount, diid.stride});
}

void cmd(cmd_args &args, const dispatch_indirect_data_type&did) {
	const type::supplier<data::buffer_type> &buffer(did.buffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer);});
	cmd(args, dispatch_indirect_type{std::ref(data::internal::get_buffer(*buffer)), did.offset});
}

void cmd(cmd_args &args, const copy_data_buffer_type&cdb) {
	const type::supplier<data::buffer_type> &buffer(cdb.srcBuffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer);});
	cmd(args, copy_buffer_type{std::ref(data::internal::get_buffer(*buffer)), cdb.dstBuffer, cdb.regions});
}

void cmd(cmd_args &args, const copy_data_buffer_to_image_type&cdbti) {
	const type::supplier<data::buffer_type> &buffer(cdbti.srcBuffer);
	args.pre_execute_callbacks.add([buffer](queue::queue_type &queue) {data::flush(queue, *buffer);});
	cmd(args, copy_buffer_to_image_type{std::ref(data::internal::get_buffer(*buffer)), cdbti.dstImage, cdbti.dstImageLayout, cdbti.regions});
}

}  // namespace internal

}  // namespace command_buffer

namespace descriptor_set {

buffer_info_data_type buffer_info(const type::supplier<data::buffer_type> &buffer, VkDeviceSize offset, VkDeviceSize range) {
	return buffer_info_data_type{ buffer, offset, range };
}

buffer_info_data_type buffer_info(const type::supplier<data::buffer_type> &buffer) {
	const std::size_t size(type::size(data::internal::get_serialize(*buffer)));
	return buffer_info_data_type{ buffer, 0, size };
}

namespace internal {

void add(update_storage &storage, const write_buffer_data_type &wbdt) {
	std::vector<buffer_info_type> buffer_infos;
	buffer_infos.reserve(wbdt.buffers.size());
	for (std::size_t i = 0; i < wbdt.buffers.size(); ++i) {
		const buffer_info_data_type &buffer(wbdt.buffers[i]);
		buffer_infos.push_back(buffer_info_type{
			std::ref(data::internal::get_buffer(*buffer.buffer)),
			buffer.offset, buffer.range});
		const type::supplier<data::buffer_type> &buf(buffer.buffer);
		wbdt.dst_set->pre_execute_callbacks.put(vcc::internal::bind_point_type{wbdt.dst_binding, uint32_t(wbdt.dst_array_element + i)},
			[buf](queue::queue_type &queue) {data::flush(queue, *buf); });
	}
	add(storage, write_buffer_type{ wbdt.dst_set, wbdt.dst_binding, wbdt.dst_array_element, wbdt.descriptor_type, std::move(buffer_infos) });
}

void count(update_storage &storage, const write_buffer_data_type &wbdt) {
	++storage.write_sets_size;
	storage.buffer_info_size += wbdt.buffers.size();
}

}

}  // namespace descriptor_set

namespace memory {
namespace internal {

VkMemoryRequirements get_memory_requirements(const data::buffer_type &buffer) {
	return get_memory_requirements(data::internal::get_buffer(buffer));
}

void bind(const type::supplier<memory_type> &memory, VkDeviceSize offset, data::buffer_type &buffer) {
	return bind(memory, offset, data::internal::get_buffer(buffer));
}

}  // namespace internal
}  // namespace memory

}  // namespace vcc


