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
#include <vcc/descriptor_pool.h>

namespace vcc {
namespace descriptor_pool {

descriptor_pool_type create(const type::supplier<device::device_type> &device,
		VkDescriptorPoolCreateFlags flags,
		uint32_t maxSets,
		const std::vector<VkDescriptorPoolSize> &poolSizes) {
	VkDescriptorPoolCreateInfo create = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, NULL};
	create.flags = flags;
	create.maxSets = maxSets;
	create.poolSizeCount = (uint32_t) poolSizes.size();
	create.pPoolSizes = poolSizes.empty() ? NULL : &poolSizes.front();
	VkDescriptorPool descriptor_pool;
	VKCHECK(vkCreateDescriptorPool(internal::get_instance(*device), &create,
		NULL, &descriptor_pool));
	return descriptor_pool_type(descriptor_pool, device);
}

}  // namespace descriptor_pool
}  // namespace vcc


