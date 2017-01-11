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
#include <vcc/memory.h>
#include <vcc/physical_device.h>

namespace vcc {
namespace memory {

memory_type memory_type::allocate(const type::supplier<const device::device_type> &device,
		VkDeviceSize allocationSize, uint32_t memoryTypeIndex, VkMemoryType type) {
	VkMemoryAllocateInfo allocate = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL};
	allocate.allocationSize = allocationSize;
	allocate.memoryTypeIndex = memoryTypeIndex;
	VkDeviceMemory memory;
	VKCHECK(vkAllocateMemory(vcc::internal::get_instance(*device), &allocate, NULL,
		&memory));
	return memory_type(memory, device, allocationSize, type);
}

namespace internal {

VkMemoryRequirements get_memory_requirements(const image::image_type &image) {
	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(vcc::internal::get_instance(
		*vcc::internal::get_parent(image)), vcc::internal::get_instance(image),
		&requirements);
	return requirements;
}

void bind(const type::supplier<const memory_type> &memory, VkDeviceSize offset,
		image::image_type &image) {
	{
		std::lock_guard<std::mutex> lock(vcc::internal::get_mutex(image));
		VKCHECK(vkBindImageMemory(
			vcc::internal::get_instance(*vcc::internal::get_parent(image)),
			vcc::internal::get_instance(image),
			vcc::internal::get_instance(*memory), offset));
	}
	vcc::internal::get_memory(image) = memory;
	vcc::internal::get_offset(image) = offset;
}

VkMemoryRequirements get_memory_requirements(
		const buffer::buffer_type &buffer) {
	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(
		vcc::internal::get_instance(*vcc::internal::get_parent(buffer)),
		vcc::internal::get_instance(buffer), &requirements);
	return requirements;
}

void bind(const type::supplier<const memory_type> &memory, VkDeviceSize offset,
		buffer::buffer_type &buffer) {
	{
		std::lock_guard<std::mutex> lock(vcc::internal::get_mutex(buffer));
		VKCHECK(vkBindBufferMemory(
			vcc::internal::get_instance(*vcc::internal::get_parent(buffer)),
			vcc::internal::get_instance(buffer),
			vcc::internal::get_instance(*memory), offset));
	}
	vcc::internal::get_memory(buffer) = memory;
	vcc::internal::get_offset(buffer) = offset;
}

VkMemoryRequirements get_memory_requirements(const input_buffer::input_buffer_type &buffer) {
	return get_memory_requirements(input_buffer::internal::get_buffer(buffer));
}

void bind(const type::supplier<const memory_type> &memory, VkDeviceSize offset,
		input_buffer::input_buffer_type &buffer) {
	bind(memory, offset, input_buffer::internal::get_buffer(buffer));
}

}  // namespace internal

map_type::~map_type() {
	if (memory) {
		std::lock_guard<std::mutex> lock(vcc::internal::get_mutex(*memory));
		vkUnmapMemory(
			vcc::internal::get_instance(*vcc::internal::get_parent(*memory)),
			vcc::internal::get_instance(*memory));
		if (!(memory->type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			flush(*memory, offset, size);
		}
	}
}

map_type map(const type::supplier<const memory_type> &memory, VkDeviceSize offset,
		VkDeviceSize size) {
	void *data;
	{
		std::lock_guard<std::mutex> lock(vcc::internal::get_mutex(*memory));
		VKCHECK(vkMapMemory(
			vcc::internal::get_instance(*vcc::internal::get_parent(*memory)),
			vcc::internal::get_instance(*memory), offset, size, 0, &data));
	}
	return map_type(memory, offset, size, data);
}

void flush(const memory_type &memory, VkDeviceSize offset, VkDeviceSize size) {
	VkMappedMemoryRange range = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr,
		vcc::internal::get_instance(memory), offset, size };
	VKCHECK(vkFlushMappedMemoryRanges(
		vcc::internal::get_instance(*vcc::internal::get_parent(memory)), 1, &range));
}

void invalidate(const memory_type &memory, VkDeviceSize offset, VkDeviceSize size) {
	VkMappedMemoryRange range = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr,
		vcc::internal::get_instance(memory), offset, size };
	VKCHECK(vkInvalidateMappedMemoryRanges(
		vcc::internal::get_instance(*vcc::internal::get_parent(memory)), 1, &range));
}

}  // namespace memory
}  // namespace vcc
