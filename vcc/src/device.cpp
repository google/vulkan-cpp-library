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
#include <algorithm>
#include <iterator>
#include <vcc/device.h>

namespace vcc {
namespace device {

std::vector<VkDeviceQueueCreateInfo> convert(const std::vector<queue_create_info_type> &queue_create_info) {
	std::vector<VkDeviceQueueCreateInfo> converted;
	converted.reserve(queue_create_info.size());
	std::transform(queue_create_info.begin(), queue_create_info.end(), std::back_inserter(converted),
			[](const queue_create_info_type &info) {
		VkDeviceQueueCreateInfo converted_info = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, NULL, 0};
		converted_info.pQueuePriorities = info.queuePriorities.empty() ? NULL : &info.queuePriorities.front();
		converted_info.queueCount = (uint32_t)info.queuePriorities.size();
		converted_info.queueFamilyIndex = info.queueFamilyIndex;
		return converted_info;
	});
	return converted;
}

device_type create(VkPhysicalDevice physical_device, const std::vector<queue_create_info_type> &queue_create_info,
		const std::set<std::string> &layers, const std::set<std::string> &extensions,
		const VkPhysicalDeviceFeatures &features) {
	VkDeviceCreateInfo create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, NULL, 0};
	const std::vector<VkDeviceQueueCreateInfo> converted(convert(queue_create_info));
	create_info.queueCreateInfoCount = (uint32_t)converted.size();
	create_info.pQueueCreateInfos = converted.empty() ? NULL : &converted.front();
	create_info.enabledLayerCount = (uint32_t)layers.size();
	const std::vector<const char *> layer_pointers(util::to_pointers(layers));
	create_info.ppEnabledLayerNames = layers.empty() ? NULL : &layer_pointers.front();
	create_info.enabledExtensionCount = (uint32_t) extensions.size();
	const std::vector<const char *> extension_pointers(util::to_pointers(extensions));
	create_info.ppEnabledExtensionNames = extension_pointers.empty() ? NULL : &extension_pointers.front();
	create_info.pEnabledFeatures = &features;
	VkDevice device;
	VKCHECK(vkCreateDevice(physical_device, &create_info, NULL, &device));
	return device_type(device, physical_device);
}

void wait_idle(const device_type &device) {
	VKCHECK(vkDeviceWaitIdle(internal::get_instance(device)));
}

}  // namespace device
}  // namespace vcc
