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
#ifndef INPUT_BUFFER_H_
#define INPUT_BUFFER_H_

#include <type/serialize.h>
#include <vcc/buffer.h>

namespace vcc {

namespace queue {

struct queue_type;

}  // namespace queue

namespace input_buffer {
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

class input_buffer_type {
	template<typename... StorageType>
	friend input_buffer_type create(type::memory_layout,
		const type::supplier<const device::device_type> &, VkBufferCreateFlags, VkBufferUsageFlags,
		VkSharingMode, const std::vector<uint32_t> &, StorageType...);
	friend VCC_LIBRARY bool flush(const input_buffer_type &buffer);
	friend VCC_LIBRARY bool flush(const queue::queue_type &queue, const input_buffer_type &buffer);
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
	input_buffer_type() = default;
	input_buffer_type(const input_buffer_type&) = delete;
	input_buffer_type(input_buffer_type &&copy) {
		std::unique_lock<std::mutex> lock(copy.mutex);
		serialize = std::move(copy.serialize);
		buffer = std::move(copy.buffer);
	}
	input_buffer_type &operator=(const input_buffer_type&) = delete;
	input_buffer_type &operator=(input_buffer_type &&copy) {
		std::lock(mutex, copy.mutex);
		std::unique_lock<std::mutex> lock(mutex, std::adopt_lock);
		std::unique_lock<std::mutex> copy_lock(copy.mutex, std::adopt_lock);
		serialize = std::move(copy.serialize);
		buffer = std::move(copy.buffer);
		return *this;
	}

private:
	template<typename... StorageType>
	input_buffer_type(type::memory_layout layout,
		const type::supplier<const device::device_type> &device, VkBufferCreateFlags flags,
		VkBufferUsageFlags usage, VkSharingMode sharingMode,
		const std::vector<uint32_t> &queueFamilyIndices, StorageType... storages)
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
input_buffer_type create(type::memory_layout layout,
		const type::supplier<const device::device_type> &device,
		VkBufferCreateFlags flags,
		VkBufferUsageFlags usage,
		VkSharingMode sharingMode,
		const std::vector<uint32_t> &queueFamilyIndices,
		StorageType... storages) {
	return input_buffer_type(layout, device, flags, usage, sharingMode, queueFamilyIndices,
			std::forward<StorageType>(storages)...);
}

// Flushes content of the buffer to the GPU if there is data with an old revision.
VCC_LIBRARY bool flush(const input_buffer_type &buffer);

// Flushes content of the buffer to the GPU if there is data with an old revision.
// A memory barrier is pushed on the queue.
VCC_LIBRARY bool flush(const queue::queue_type &queue, input_buffer_type &buffer);

}  // namespace input_buffer
}  // namespace vcc

#endif /* INPUT_BUFFER_H_ */
