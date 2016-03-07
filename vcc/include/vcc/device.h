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
#ifndef DEVICE_H_
#define DEVICE_H_

#include <vcc/internal/raii.h>
#include <vcc/util.h>

namespace vcc {
namespace device {

struct queue_create_info_type {
	uint32_t queueFamilyIndex;
	std::vector<float> queuePriorities;
};

struct device_type
	: public internal::movable_destructible<VkDevice, vkDestroyDevice> {
	friend VCC_LIBRARY device_type create(VkPhysicalDevice physical_device,
		const std::vector<queue_create_info_type> &queue_create_info,
		const std::set<std::string> &layers,
		const std::set<std::string> &extensions,
		const VkPhysicalDeviceFeatures &features);
	friend VkPhysicalDevice get_physical_device(const device_type &device);

	device_type() = default;
	device_type(const device_type&) = delete;
	device_type(device_type&&copy) = default;
	device_type &operator=(const device_type&) = delete;
	device_type &operator=(device_type&&copy) = default;

private:
	device_type(VkDevice device, VkPhysicalDevice physical_device)
		: internal::movable_destructible<VkDevice, vkDestroyDevice>(device),
		  physical_device(physical_device) {}

	internal::handle_type<VkPhysicalDevice> physical_device;
};

VCC_LIBRARY device_type create(VkPhysicalDevice physical_device,
	const std::vector<queue_create_info_type> &queue_create_info,
	const std::set<std::string> &layers,
	const std::set<std::string> &extensions,
	const VkPhysicalDeviceFeatures &features);

VCC_LIBRARY void wait_idle(const device_type &device);

inline VkPhysicalDevice get_physical_device(const device_type &device) {
	return device.physical_device;
}

}  // namespace device
}  // namespace vcc

#endif /* DEVICE_H_ */
