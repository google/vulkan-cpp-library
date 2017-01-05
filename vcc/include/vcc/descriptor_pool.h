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
#ifndef DESCRIPTOR_POOL_H_
#define DESCRIPTOR_POOL_H_

#include <vcc/device.h>

namespace vcc {
namespace descriptor_pool {

struct descriptor_pool_type
	: public internal::movable_destructible_with_parent<VkDescriptorPool,
		const device::device_type, vkDestroyDescriptorPool> {
	friend VCC_LIBRARY descriptor_pool_type create(
		const type::supplier<const device::device_type> &, VkDescriptorPoolCreateFlags, uint32_t,
		const std::vector<VkDescriptorPoolSize> &);

	descriptor_pool_type() = default;
	descriptor_pool_type(descriptor_pool_type &&) = default;
	descriptor_pool_type(const descriptor_pool_type &) = delete;
	descriptor_pool_type &operator=(descriptor_pool_type &&) = default;
	descriptor_pool_type &operator=(const descriptor_pool_type &) = delete;

private:
	descriptor_pool_type(VkDescriptorPool instance,
		const type::supplier<const device::device_type> &parent)
		: movable_destructible_with_parent(instance, parent) {}
};

VCC_LIBRARY descriptor_pool_type create(
	const type::supplier<const device::device_type> &device,
	VkDescriptorPoolCreateFlags flags, uint32_t maxSets,
	const std::vector<VkDescriptorPoolSize> &poolSizes);

}  // namespace descriptor_pool
}  // namespace vcc

#endif /* DESCRIPTOR_POOL_H_ */
