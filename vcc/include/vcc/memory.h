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
#include <vcc/device.h>
#include <vcc/image.h>
#include <vcc/physical_device.h>

namespace vcc {
namespace memory {

struct memory_type : vcc::internal::movable_destructible_with_parent<
		VkDeviceMemory, device::device_type, vkFreeMemory> {
	friend VCC_LIBRARY memory_type allocate(
		const type::supplier<device::device_type> &device,
		VkDeviceSize allocationSize, uint32_t memoryTypeIndex);
	memory_type() = default;
	memory_type(memory_type &&instance) = default;

private:
	memory_type(VkDeviceMemory instance,
		const type::supplier<device::device_type> &parent, VkDeviceSize size)
		: vcc::internal::movable_destructible_with_parent<VkDeviceMemory,
			device::device_type, vkFreeMemory>(instance, parent),
		  size(size) {}

	VkDeviceSize size;
};

VCC_LIBRARY memory_type allocate(const type::supplier<device::device_type> &device,
	VkDeviceSize allocationSize, uint32_t memoryTypeIndex);

namespace internal {

VCC_LIBRARY VkMemoryRequirements get_memory_requirements(const image::image_type &image);
VCC_LIBRARY VkMemoryRequirements get_memory_requirements(const buffer::buffer_type &buffer);
VCC_LIBRARY void bind(const type::supplier<memory_type> &memory, VkDeviceSize offset, image::image_type &image);
VCC_LIBRARY void bind(const type::supplier<memory_type> &memory, VkDeviceSize offset, buffer::buffer_type &buffer);

template<std::size_t Index>
struct bind_t {
	template<typename... ArgsT>
	static void bind(type::supplier<memory_type> &&memory, VkDeviceSize *offsets,
			std::tuple<ArgsT...>&& args) {
		internal::bind(memory, offsets[Index - 1], std::get<Index - 1>(args));
		bind_t<Index - 1>::bind(std::forward<type::supplier<memory_type>>(memory),
				offsets,
				std::forward<std::tuple<ArgsT...>>(args));
	}
};

template<>
struct bind_t<0> {
	template<typename... ArgsT>
	static void bind(type::supplier<memory_type> &&memory, VkDeviceSize *offsets, std::tuple<ArgsT...>&& args) {}
};

}  // namespace internal

template<typename... ArgsT>
type::supplier<memory_type> bind(
		const type::supplier<device::device_type> &device,
		VkMemoryPropertyFlags propertyFlags,
		ArgsT&... args) {
	constexpr size_t num_args(sizeof...(ArgsT));
	static_assert(num_args == 1, "Using multiple arguments with bind is currently buggy");
	const VkMemoryRequirements memory_requirements[] = { internal::get_memory_requirements(args)... };
	VkDeviceSize offsets[num_args];
	offsets[0] = 0;
	uint32_t memoryTypeBits(UINT_MAX);
	for (int i = 1; i < num_args; ++i) {
		offsets[i] = offsets[i - 1] + memory_requirements[i - 1].size;
		VkDeviceSize alignment(offsets[i] % memory_requirements[i].alignment);
		if (alignment) {
			offsets[i] += memory_requirements[i].alignment - alignment;
		}
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
	std::shared_ptr<memory_type> memory(std::make_shared<memory_type>(
			allocate(device, size, memoryTypeIndex)));
	internal::bind_t<num_args>::bind(memory, offsets, std::tie(args...));
	return memory;
}

struct map_type {
	map_type() = delete;
	map_type(const map_type&) = delete;
	map_type(map_type &&copy) : memory(copy.memory), data(copy.data) {
		copy.memory = type::supplier<memory_type>();
		copy.data = nullptr;
	}
	VCC_LIBRARY ~map_type();
	explicit map_type(const type::supplier<memory_type> &memory)
		: memory(memory) {}
	type::supplier<memory_type> memory;

	void *data;
};

// Returns a map_type which will automatically unmap the memory, RAII style.
// map_type::data is the pointer to the area where the memory is mapped.
VCC_LIBRARY map_type map(const type::supplier<memory_type> &memory, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

}  // namespace memory
}  // namespace vcc

#endif /* MEMORY_H_ */
