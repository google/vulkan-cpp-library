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
#ifndef COMMAND_POOL_H_
#define COMMAND_POOL_H_

#include <vcc/device.h>

namespace vcc {
namespace command_pool {

struct command_pool_type
	: public internal::movable_destructible_with_parent<VkCommandPool,
	device::device_type, vkDestroyCommandPool> {
	friend VCC_LIBRARY command_pool_type create(
		const type::supplier<device::device_type> &device,
		VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);
	command_pool_type() = default;
	command_pool_type(command_pool_type &&) = default;
	command_pool_type(const command_pool_type &) = delete;
	command_pool_type &operator=(command_pool_type &&) = default;
	command_pool_type &operator=(const command_pool_type &) = delete;

private:
	command_pool_type(VkCommandPool instance,
		const type::supplier<device::device_type> &parent)
		: internal::movable_destructible_with_parent<VkCommandPool,
		device::device_type, vkDestroyCommandPool>(instance, parent) {}
};

VCC_LIBRARY command_pool_type create(
	const type::supplier<device::device_type> &device,
	VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);

}  // namespace command_pool
}  // namespace vcc

#endif /* COMMAND_POOL_H_ */
