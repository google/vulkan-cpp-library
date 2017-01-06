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
#include <vcc/command_pool.h>

namespace vcc {
namespace command_pool {

command_pool_type create(const type::supplier<const device::device_type> &device,
		VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex) {
	VkCommandPoolCreateInfo create = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL};
	create.flags = flags;
	create.queueFamilyIndex = queueFamilyIndex;
	VkCommandPool pool;
	VKCHECK(vkCreateCommandPool(internal::get_instance(*device), &create, NULL,
		&pool));
	return command_pool_type(pool, device);
}

}  // namespace command_pool
}  // namespace vcc
