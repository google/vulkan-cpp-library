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

memory_type allocate(const type::supplier<device::device_type> &device,
		VkDeviceSize allocationSize, uint32_t memoryTypeIndex) {
	VkMemoryAllocateInfo allocate = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL};
	allocate.allocationSize = allocationSize;
	allocate.memoryTypeIndex = memoryTypeIndex;
	VkDeviceMemory memory;
	VKCHECK(vkAllocateMemory(vcc::internal::get_instance(*device), &allocate, NULL,
		&memory));
	return memory_type(memory, device, allocationSize);
}

namespace internal {

VkMemoryRequirements get_memory_requirements(const image::image_type &image) {
	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(vcc::internal::get_instance(
		*vcc::internal::get_parent(image)), vcc::internal::get_instance(image),
		&requirements);
	return requirements;
}

void bind(const type::supplier<memory_type> &memory, VkDeviceSize offset,
		image::image_type &image) {
	VKCHECK(vkBindImageMemory(
		vcc::internal::get_instance(*vcc::internal::get_parent(image)),
		vcc::internal::get_instance(image),
		vcc::internal::get_instance(*memory), offset));
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

void bind(const type::supplier<memory_type> &memory, VkDeviceSize offset,
		buffer::buffer_type &buffer) {
	VKCHECK(vkBindBufferMemory(
		vcc::internal::get_instance(*vcc::internal::get_parent(buffer)),
		vcc::internal::get_instance(buffer),
		vcc::internal::get_instance(*memory), offset));
	vcc::internal::get_memory(buffer) = memory;
	vcc::internal::get_offset(buffer) = offset;
}

}  // namespace internal

map_type::~map_type() {
	if (memory) {
		vkUnmapMemory(
			vcc::internal::get_instance(*vcc::internal::get_parent(*memory)),
			vcc::internal::get_instance(*memory));
		// TODO(gardell): Flush depending on memory type.
	}
}

map_type map(const type::supplier<memory_type> &memory, VkDeviceSize offset,
		VkDeviceSize size) {
	map_type mapped(memory);
	VKCHECK(vkMapMemory(
		vcc::internal::get_instance(*vcc::internal::get_parent(*mapped.memory)),
		vcc::internal::get_instance(*mapped.memory), offset, size, 0, &mapped.data));
	return std::move(mapped);
}

}  // namespace memory
}  // namespace vcc
