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
#include <vcc/physical_device.h>

namespace vcc {
namespace physical_device {

std::vector<VkPhysicalDevice> enumerate(const instance::instance_type &instance) {
	uint32_t count;
	VKCHECK(vkEnumeratePhysicalDevices(internal::get_instance(instance), &count, NULL));
	std::vector<VkPhysicalDevice> devices(count, VK_NULL_HANDLE);
	VKCHECK(vkEnumeratePhysicalDevices(internal::get_instance(instance),
		&count, devices.data()));
	return std::move(devices);
}

VkPhysicalDeviceMemoryProperties memory_properties(VkPhysicalDevice physical_device) {
	VkPhysicalDeviceMemoryProperties properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);
	return properties;
}

std::vector<VkQueueFamilyProperties> queue_famility_properties(VkPhysicalDevice physical_device) {
	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, NULL);
	std::vector<VkQueueFamilyProperties> properties(count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, &properties.front());
	return std::move(properties);
}

uint32_t get_queue_family_properties_with_flag(const std::vector<VkQueueFamilyProperties> &properties, VkQueueFlags flags) {
	for (std::size_t i = 0; i < properties.size(); ++i) {
		const VkQueueFamilyProperties &p(properties[i]);
		if ((p.queueFlags & flags) == flags) {
			return uint32_t(i);
		}
	}
	throw vcc_exception("No queue family found with the given properties.");
}

}  // namespace physical_device
}  // namespace vcc
