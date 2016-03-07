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
#ifndef PHYSICAL_DEVICE_H_
#define PHYSICAL_DEVICE_H_

#include <type/supplier.h>
#include <vcc/util.h>
#include <vcc/instance.h>

namespace vcc {
namespace physical_device {

VCC_LIBRARY std::vector<VkPhysicalDevice> enumerate(
	const instance::instance_type &instance);

inline VkPhysicalDeviceProperties properties(
		VkPhysicalDevice physical_device) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physical_device, &properties);
	return properties;
}

inline VkPhysicalDeviceFeatures features(VkPhysicalDevice physical_device) {
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(physical_device, &features);
	return features;
}

inline VkFormatProperties format_properties(VkPhysicalDevice physical_device,
		VkFormat format) {
	VkFormatProperties properties;
	vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);
	return properties;
}

VCC_LIBRARY VkPhysicalDeviceMemoryProperties memory_properties(
	VkPhysicalDevice physical_device);

// TODO: More get methods

VCC_LIBRARY std::vector<VkQueueFamilyProperties> queue_famility_properties(
	VkPhysicalDevice physical_device);

VCC_LIBRARY uint32_t get_queue_family_properties_with_flag(
	const std::vector<VkQueueFamilyProperties> &properties,
	VkQueueFlags flags);

}  // namespace physical_device
}  // namespace vcc


#endif /* PHYSICAL_DEVICE_H_ */
