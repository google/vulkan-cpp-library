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
#ifndef DATA_BUFFER_H_
#define DATA_BUFFER_H_

#include <type/serialize.h>
#include <vcc/buffer.h>
#include <vcc/command.h>
#include <vcc/command_buffer.h>

namespace vcc {

namespace queue {

struct queue_type;

}  // namespace queue

namespace data {
namespace internal {

template<typename T>
auto get_mutex(const T &value)->decltype(value.mutex)& {
	return value.mutex;
}

template<typename T>
auto get_buffer(T &value)->decltype(value.buffer)& {
	return value.buffer;
}

template<typename T>
auto get_serialize(T &value)->decltype(value.serialize)& {
	return value.serialize;
}

template<typename T>
auto get_buffer(const T &value)->const decltype(value.buffer)& {
	return value.buffer;
}

template<typename T>
auto get_serialize(const T &value)->const decltype(value.serialize)& {
	return value.serialize;
}

}  // namespace internal

/**
 * data::buffer_type takes a set of data::array_view_type, for example data::vec3_array,
 * or lambdas providing std::vector or std::array.
 * and manages revision counting on data types to propagate its content to a gpu buffer object.
 *
 * By modifying data::*_array types using their mutable interfaces, their content is lazily
 * propagated on all gpu buffers using this buffer.
 */

class buffer_type {
	template<typename... StorageType>
	friend buffer_type create(type::memory_layout layout,
		const type::supplier<device::device_type> &device,
		VkBufferCreateFlags flags,
		VkBufferUsageFlags usage,
		VkSharingMode sharingMode,
		const std::vector<uint32_t> &queueFamilyIndices,
		StorageType... storages);
	friend VCC_LIBRARY void flush(buffer_type &buffer);
	friend VCC_LIBRARY void flush(queue::queue_type &queue,
		buffer_type &buffer);
	template<typename U>
	friend auto internal::get_mutex(const U &value)->decltype(value.mutex)&;
	template<typename U>
	friend auto internal::get_buffer(U &value)->decltype(value.buffer)&;
	template<typename U>
	friend auto internal::get_serialize(U &value)->decltype(value.serialize)&;
	template<typename U>
	friend auto internal::get_buffer(const U &value)->const decltype(value.buffer)&;
	template<typename U>
	friend auto internal::get_serialize(const U &value)->const decltype(value.serialize)&;
public:
	buffer_type() = default;
	buffer_type(const buffer_type&) = delete;
	buffer_type(buffer_type &&copy) {
		std::unique_lock<std::mutex> lock(copy.mutex);
		serialize = std::move(copy.serialize);
		buffer = std::move(copy.buffer);
	}
	buffer_type &operator=(const buffer_type&) = delete;
	buffer_type &operator=(buffer_type &&copy) {
		std::lock(mutex, copy.mutex);
		std::unique_lock<std::mutex> lock(mutex, std::adopt_lock);
		std::unique_lock<std::mutex> copy_lock(copy.mutex, std::adopt_lock);
		serialize = std::move(copy.serialize);
		buffer = std::move(copy.buffer);
		return *this;
	}

private:
	template<typename... StorageType>
	buffer_type(type::memory_layout layout,
		const type::supplier<device::device_type> &device,
		VkBufferCreateFlags flags,
		VkBufferUsageFlags usage,
		VkSharingMode sharingMode,
		const std::vector<uint32_t> &queueFamilyIndices,
		StorageType... storages)
		: serialize(type::make_serialize(layout,
			std::forward<StorageType>(storages)...)),
		  buffer(std::forward<buffer::buffer_type>(
			  buffer::create(device, flags, type::size(serialize), usage,
				  sharingMode, queueFamilyIndices))) {}

	type::serialize_type serialize;
	buffer::buffer_type buffer;
	mutable std::mutex mutex;
};

/*
 * Creates a buffer_type containing the given data.
 * Notice: The buffer must be bound to memory before usage.
 */
template<typename... StorageType>
buffer_type create(type::memory_layout layout,
		const type::supplier<device::device_type> &device,
		VkBufferCreateFlags flags,
		VkBufferUsageFlags usage,
		VkSharingMode sharingMode,
		const std::vector<uint32_t> &queueFamilyIndices,
		StorageType... storages) {
	return buffer_type(layout, device, flags, usage, sharingMode, queueFamilyIndices,
			std::forward<StorageType>(storages)...);
}

// Flushes content of the buffer to the GPU if there is data with an old revision.
VCC_LIBRARY void flush(buffer_type &buffer);

// Flushes content of the buffer to the GPU if there is data with an old revision.
// A memory barrier is pushed on the queue.
VCC_LIBRARY void flush(queue::queue_type &queue, buffer_type &buffer);

}  // namespace data

/*
 * Interface to comply with command_buffer interface for data::buffer_type.
 */
namespace command {

struct bind_index_data_buffer_type {
	type::supplier<data::buffer_type> buffer;
	VkDeviceSize offset;
	VkIndexType indexType;
};

inline bind_index_data_buffer_type bind_index_data_buffer(
		const type::supplier<data::buffer_type> &buffer,
		VkDeviceSize offset, VkIndexType indexType) {
	return bind_index_data_buffer_type{buffer, offset, indexType};
}

struct bind_vertex_data_buffers_type {
	std::vector<type::supplier<data::buffer_type>> buffers;
	std::vector<VkDeviceSize> offsets;
};

inline bind_vertex_data_buffers_type bind_vertex_data_buffers(
		const std::vector<type::supplier<data::buffer_type>> &buffers,
		const std::vector<VkDeviceSize> &offsets) {
	return bind_vertex_data_buffers_type{buffers, offsets};
}

struct draw_indirect_data_type {
	type::supplier<data::buffer_type> buffer;
	VkDeviceSize offset;
	uint32_t drawCount, stride;
};

inline draw_indirect_data_type draw_indirect_data(
		const type::supplier<data::buffer_type> &buffer,
		VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
	return draw_indirect_data_type{buffer, offset, drawCount, stride};
}

struct draw_indexed_indirect_data_type {
	type::supplier<data::buffer_type> buffer;
	VkDeviceSize offset;
	uint32_t drawCount, stride;
};

inline draw_indexed_indirect_data_type draw_indexed_indirect_data(
		const type::supplier<data::buffer_type> &buffer,
		VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
	return draw_indexed_indirect_data_type{buffer, offset, drawCount, stride};
}

struct dispatch_indirect_data_type {
	type::supplier<data::buffer_type> buffer;
	VkDeviceSize offset;
};

inline dispatch_indirect_data_type dispatch_indirect_data(
		const type::supplier<data::buffer_type> &buffer,
		VkDeviceSize offset) {
	return dispatch_indirect_data_type{buffer, offset};
}

struct copy_data_buffer_type {
	type::supplier<data::buffer_type> srcBuffer;
	type::supplier<buffer::buffer_type> dstBuffer;
	std::vector<VkBufferCopy> regions;
};

inline copy_data_buffer_type copy_data_buffer(
		const type::supplier<data::buffer_type> &srcBuffer,
		const type::supplier<buffer::buffer_type> &dstBuffer,
		const std::vector<VkBufferCopy> &regions) {
	return copy_data_buffer_type{srcBuffer, dstBuffer, regions};
}

struct copy_data_buffer_to_image_type {
	type::supplier<data::buffer_type> srcBuffer;
	type::supplier<image::image_type> dstImage;
	VkImageLayout dstImageLayout;
	std::vector<VkBufferImageCopy> regions;
};

inline copy_data_buffer_to_image_type copy_data_buffer_to_image(
		const type::supplier<data::buffer_type> &srcBuffer,
		const type::supplier<image::image_type> &dstImage,
		VkImageLayout dstImageLayout,
		const std::vector<VkBufferImageCopy> &regions) {
	return copy_data_buffer_to_image_type{
		srcBuffer, dstImage,
		dstImageLayout, regions};
}

namespace internal {

VCC_LIBRARY void cmd(cmd_args &, const bind_index_data_buffer_type&);
VCC_LIBRARY void cmd(cmd_args &, const bind_vertex_data_buffers_type&);
VCC_LIBRARY void cmd(cmd_args &, const draw_indirect_data_type&);
VCC_LIBRARY void cmd(cmd_args &, const draw_indexed_indirect_data_type&);
VCC_LIBRARY void cmd(cmd_args &, const dispatch_indirect_data_type&);
VCC_LIBRARY void cmd(cmd_args &, const copy_data_buffer_type&);
VCC_LIBRARY void cmd(cmd_args &, const copy_data_buffer_to_image_type&);

}  // namespace internal

inline buffer_memory_barrier_type buffer_memory_barrier(
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
	uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex,
	const type::supplier<data::buffer_type> &buffer,
	VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) {
	return buffer_memory_barrier_type{
		srcAccessMask, dstAccessMask,
		srcQueueFamilyIndex, dstQueueFamilyIndex,
		std::ref(data::internal::get_buffer(*buffer)), offset, size
	};
}

}  // namespace command

namespace descriptor_set {

struct buffer_info_data_type {
	type::supplier<data::buffer_type> buffer;
	VkDeviceSize offset, range;
};

VCC_LIBRARY buffer_info_data_type buffer_info(
	const type::supplier<data::buffer_type> &buffer,
	VkDeviceSize offset, VkDeviceSize range);
VCC_LIBRARY buffer_info_data_type buffer_info(
	const type::supplier<data::buffer_type> &buffer);

struct write_buffer_data_type {
	type::supplier<descriptor_set_type> dst_set;
	uint32_t dst_binding, dst_array_element;
	// Must be any of the following
	// VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_
	// STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC or VK_DESCRIPTOR_
	// TYPE_STORAGE_BUFFER_DYNAMIC
	VkDescriptorType descriptor_type;
	std::vector<buffer_info_data_type> buffers;
};

inline write_buffer_data_type write_buffer(
		const type::supplier<descriptor_set_type> &dst_set,
		uint32_t dst_binding, uint32_t dst_array_element,
		VkDescriptorType descriptor_type,
		const std::vector<buffer_info_data_type> &buffers) {
	return write_buffer_data_type{ dst_set, dst_binding,
		dst_array_element, descriptor_type, buffers };
}

namespace internal {

VCC_LIBRARY void add(update_storage &storage, const write_buffer_data_type &);

VCC_LIBRARY void count(update_storage &storage,
	const write_buffer_data_type &);

}  // namespace internal

}  // namespace descriptor_set

namespace memory {

struct memory_type;

namespace internal {

VCC_LIBRARY VkMemoryRequirements get_memory_requirements(
	const data::buffer_type &buffer);
VCC_LIBRARY void bind(const type::supplier<memory_type> &memory,
	VkDeviceSize offset, data::buffer_type &buffer);

}  // namespace internal
}  // namespace memory

}  // namespace vcc

#endif /* DATA_BUFFER_H_ */
