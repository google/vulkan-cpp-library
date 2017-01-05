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
#include <vcc/buffer.h>

namespace vcc {
namespace buffer {

buffer_type create(const type::supplier<const device::device_type> &device,
		VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage,
		VkSharingMode sharingMode, const std::vector<uint32_t> &queueFamilyIndices) {
	VkBufferCreateInfo create = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL};
	create.flags = flags;
	create.size = size;
	create.usage = usage;
	create.sharingMode = sharingMode;
	create.queueFamilyIndexCount = (uint32_t) queueFamilyIndices.size();
	create.pQueueFamilyIndices = queueFamilyIndices.empty() ? NULL : &queueFamilyIndices.front();
	VkBuffer buffer;
	VKCHECK(vkCreateBuffer(internal::get_instance(*device), &create, NULL, &buffer));
	return buffer_type(buffer, device);
}

}  // namespace buffer
}  // namespace vcc


