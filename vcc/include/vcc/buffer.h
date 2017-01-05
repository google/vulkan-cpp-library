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
#ifndef BUFFER_H_
#define BUFFER_H_

#include <vcc/device.h>

namespace vcc {
namespace memory {
	struct memory_type;
}  // namespace memory

namespace buffer {

struct buffer_type
	: public internal::movable_destructible_with_parent_and_memory<VkBuffer,
		const device::device_type, const memory::memory_type, vkDestroyBuffer> {
	friend VCC_LIBRARY buffer_type create(
		const type::supplier<const device::device_type> &device,
		VkBufferCreateFlags flags, VkDeviceSize size,
		VkBufferUsageFlags usage, VkSharingMode sharingMode,
		const std::vector<uint32_t> &queueFamilyIndices);

	buffer_type() = default;
	buffer_type(buffer_type &&) = default;
	buffer_type(const buffer_type &) = delete;
	buffer_type &operator=(buffer_type &&) = default;
	buffer_type &operator=(const buffer_type &) = delete;

private:
	buffer_type(VkBuffer instance, const type::supplier<const device::device_type> &parent)
		: movable_destructible_with_parent_and_memory(instance, parent) {}
};

VCC_LIBRARY buffer_type create(
	const type::supplier<const device::device_type> &device,
	VkBufferCreateFlags flags, VkDeviceSize size,
	VkBufferUsageFlags usage, VkSharingMode sharingMode,
	const std::vector<uint32_t> &queueFamilyIndices);

}  // namespace buffer
}  // namespace vcc

#endif /* BUFFER_H_ */
