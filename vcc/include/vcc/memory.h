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
#ifndef MEMORY_H_
#define MEMORY_H_

#include <climits>
#include <numeric>
#include <vcc/buffer.h>
#include <vcc/input_buffer.h>
#include <vcc/device.h>
#include <vcc/image.h>
#include <vcc/physical_device.h>

namespace vcc {
namespace memory {

struct memory_type : vcc::internal::movable_destructible_with_parent<
		VkDeviceMemory, const device::device_type, vkFreeMemory> {

	template<typename... ArgsT>
	friend type::supplier<const memory_type> bind(
		const type::supplier<const device::device_type> &device,
		VkMemoryPropertyFlags propertyFlags, ArgsT&... args);
	friend struct map_type;

	memory_type() = default;
	memory_type(memory_type &&) = default;

private:
	VCC_LIBRARY static memory_type allocate(
		const type::supplier<const device::device_type> &device, VkDeviceSize allocationSize,
		uint32_t memoryTypeIndex, VkMemoryType type);

	memory_type(VkDeviceMemory instance, const type::supplier<const device::device_type> &parent,
		VkDeviceSize size, VkMemoryType type)
		: movable_destructible_with_parent(instance, parent), size(size), type(type) {}

	VkDeviceSize size;
	VkMemoryType type;
};

namespace internal {

VCC_LIBRARY VkMemoryRequirements get_memory_requirements(const image::image_type &image);
VCC_LIBRARY VkMemoryRequirements get_memory_requirements(const buffer::buffer_type &buffer);
VCC_LIBRARY void bind(const type::supplier<const memory_type> &memory,
	VkDeviceSize offset, image::image_type &image);
VCC_LIBRARY void bind(const type::supplier<const memory_type> &memory,
	VkDeviceSize offset, buffer::buffer_type &buffer);
VCC_LIBRARY VkMemoryRequirements get_memory_requirements(
	const input_buffer::input_buffer_type &buffer);
VCC_LIBRARY void bind(const type::supplier<const memory_type> &memory,
	VkDeviceSize offset, input_buffer::input_buffer_type &buffer);

template<std::size_t Index>
struct bind_t {
	template<typename... ArgsT>
	static void bind(const type::supplier<const memory_type> &memory, VkDeviceSize *offsets,
			std::tuple<ArgsT...>&& args) {
		internal::bind(memory, offsets[Index - 1], std::get<Index - 1>(args));
		bind_t<Index - 1>::bind(memory, offsets, std::forward<std::tuple<ArgsT...>>(args));
	}
};

template<>
struct bind_t<0> {
	template<typename... ArgsT>
	static void bind(const type::supplier<const memory_type> &memory, VkDeviceSize *offsets,
		std::tuple<ArgsT...>&& args) {}
};

}  // namespace internal

template<typename... ArgsT>
type::supplier<const memory_type> bind(
		const type::supplier<const device::device_type> &device,
		VkMemoryPropertyFlags propertyFlags,
		ArgsT&... args) {
	constexpr size_t num_args(sizeof...(ArgsT));
	const VkMemoryRequirements memory_requirements[] = { internal::get_memory_requirements(args)... };
	VkDeviceSize offsets[num_args];
	offsets[0] = 0;
	uint32_t memoryTypeBits(memory_requirements[0].memoryTypeBits);
	for (int i = 1; i < num_args; ++i) {
		auto alignment(memory_requirements[i].alignment);
		offsets[i] = offsets[i - 1] + memory_requirements[i - 1].size;
		offsets[i] += (alignment - (offsets[i] % alignment)) % alignment;
		memoryTypeBits &= memory_requirements[i].memoryTypeBits;
	}
	const VkDeviceSize size = offsets[num_args - 1] + memory_requirements[num_args - 1].size;
	if (!memoryTypeBits) {
		throw vcc_exception("No memoryTypeBits for all given storage.");
	}
	uint32_t memoryTypeIndex;
	const VkPhysicalDeviceMemoryProperties memory_properties(
		vcc::physical_device::memory_properties(device::get_physical_device(*device)));
	for (memoryTypeIndex = 0; memoryTypeIndex < memory_properties.memoryTypeCount; ++memoryTypeIndex) {
		if ((memoryTypeBits & 1) == 1
			&& (memory_properties.memoryTypes[memoryTypeIndex].propertyFlags & propertyFlags) == propertyFlags) {
			break;
		}
		memoryTypeBits >>= 1;
	}
	if (memoryTypeIndex == memory_properties.memoryTypeCount) {
		throw vcc_exception("Failed to find valid memoryTypeBits that fits the propertyFlags");
	}
	std::shared_ptr<memory_type> memory(std::make_shared<memory_type>(memory_type::allocate(device,
		size, memoryTypeIndex, memory_properties.memoryTypes[memoryTypeIndex])));
	internal::bind_t<num_args>::bind(memory, offsets, std::tie(args...));
	return memory;
}

struct map_type {
	map_type() = delete;
	map_type(const map_type&) = delete;
	map_type(map_type &&copy)
		: memory(copy.memory), offset(copy.offset), size(copy.size), data(copy.data) {
		copy.memory = type::supplier<const memory_type>();
		copy.data = nullptr;
		copy.offset = copy.size = 0;
	}
	VCC_LIBRARY ~map_type();
	map_type(const type::supplier<const memory_type> &memory, VkDeviceSize offset,
		VkDeviceSize size, void *data) : memory(memory) , offset(offset), size(size), data(data) {}
	type::supplier<const memory_type> memory;

	VkDeviceSize offset, size;
	void *data;
};

// Returns a map_type which will automatically unmap the memory, RAII style.
// map_type::data is the pointer to the area where the memory is mapped.
VCC_LIBRARY map_type map(const type::supplier<const memory_type> &memory, VkDeviceSize offset = 0,
	VkDeviceSize size = VK_WHOLE_SIZE);

VCC_LIBRARY void flush(const memory_type &memory, VkDeviceSize offset = 0,
	VkDeviceSize size = VK_WHOLE_SIZE);
VCC_LIBRARY void invalidate(const memory_type &memory, VkDeviceSize offset = 0,
	VkDeviceSize size = VK_WHOLE_SIZE);

}  // namespace memory
}  // namespace vcc

#endif /* MEMORY_H_ */
