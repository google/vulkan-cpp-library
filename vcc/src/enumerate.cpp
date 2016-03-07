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
#include <cstring>
#include <vcc/enumerate.h>
#include <vector>

namespace vcc {
namespace enumerate {

std::vector<VkLayerProperties> instance_layer_properties() {
	uint32_t count;
	VKCHECK(vkEnumerateInstanceLayerProperties(&count, NULL));
	std::vector<VkLayerProperties> properties(count);
	VKCHECK(vkEnumerateInstanceLayerProperties(&count, &properties.front()));
	return std::move(properties);
}

std::vector<VkLayerProperties> device_layer_properties(VkPhysicalDevice device) {
	uint32_t count;
	VKCHECK(vkEnumerateDeviceLayerProperties(device, &count, NULL));
	std::vector<VkLayerProperties> properties(count);
	VKCHECK(vkEnumerateDeviceLayerProperties(device, &count, &properties.front()));
	return std::move(properties);
}

bool contains_all(const std::vector<VkLayerProperties> &layers, const std::set<std::string> &required) {
	std::size_t found(0);
	for (const VkLayerProperties &layer: layers) {
		found += required.count(layer.layerName);
	}
	return found == required.size();
}

std::vector<std::string> filter(const std::vector<VkLayerProperties> &layers, const std::set<std::string> &optional) {
	std::vector<std::string> available;
	available.reserve(optional.size());
	for (const VkLayerProperties &layer : layers) {
		if (optional.count(layer.layerName)) {
			available.push_back(layer.layerName);
		}
	}
	return std::move(available);
}

bool contains_all(const std::vector<VkExtensionProperties> &extensions, const std::set<std::string> &required) {
	std::size_t found(0);
	for (const VkExtensionProperties &extension : extensions) {
		found += required.count(extension.extensionName);
	}
	return found == required.size();
}

std::vector<std::string> filter(const std::vector<VkExtensionProperties> &extensions, const std::set<std::string> &optional) {
	std::vector<std::string> available;
	available.reserve(optional.size());
	for (const VkExtensionProperties &extension : extensions) {
		if (optional.count(extension.extensionName)) {
			available.push_back(extension.extensionName);
		}
	}
	return std::move(available);
}

std::vector<VkExtensionProperties> instance_extension_properties(const std::string &layer_name) {
	uint32_t count;
	VKCHECK(vkEnumerateInstanceExtensionProperties(layer_name.empty() ? NULL : layer_name.c_str(), &count, NULL));
	std::vector<VkExtensionProperties> properties(count);
	VKCHECK(vkEnumerateInstanceExtensionProperties(layer_name.empty() ? NULL : layer_name.c_str(), &count, &properties.front()));
	return std::move(properties);
}

std::vector<VkExtensionProperties> device_extension_properties(VkPhysicalDevice device, const std::string &layer_name) {
	uint32_t count;
	VKCHECK(vkEnumerateDeviceExtensionProperties(device, layer_name.empty() ? NULL : layer_name.c_str(), &count, NULL));
	std::vector<VkExtensionProperties> properties(count);
	VKCHECK(vkEnumerateDeviceExtensionProperties(device, layer_name.empty() ? NULL : layer_name.c_str(), &count, &properties.front()));
	return std::move(properties);
}

}
}
