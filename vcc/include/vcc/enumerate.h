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
#ifndef _ENUMERATE_H_
#define _ENUMERATE_H_

#include <vcc/util.h>
#include <set>
#include <vector>

namespace vcc {
namespace enumerate {

VCC_LIBRARY std::vector<VkLayerProperties> instance_layer_properties();
VCC_LIBRARY std::vector<VkLayerProperties> device_layer_properties(
	VkPhysicalDevice device);

VCC_LIBRARY std::vector<VkExtensionProperties> instance_extension_properties(
	const std::string &layer_name);
VCC_LIBRARY std::vector<VkExtensionProperties> device_extension_properties(
	VkPhysicalDevice device, const std::string &layer_name);

// TODO(gardell): Possibly contain version, not just string
VCC_LIBRARY bool contains_all(const std::vector<VkLayerProperties> &layers,
	const std::set<std::string> &required);

VCC_LIBRARY std::vector<std::string> filter(
	const std::vector<VkLayerProperties> &layers,
	const std::set<std::string> &optional);

// TODO(gardell): Possibly contain version, not just string
VCC_LIBRARY bool contains_all(
	const std::vector<VkExtensionProperties> &extensions,
	const std::set<std::string> &required);

VCC_LIBRARY std::vector<std::string> filter(
	const std::vector<VkExtensionProperties> &extensions,
	const std::set<std::string> &optional);

}  // namespace enumerate
}  // namespace vcc

#endif /* _ENUMERATE_H_ */
