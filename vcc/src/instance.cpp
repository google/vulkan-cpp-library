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
#include <vcc/instance.h>

namespace vcc {
namespace instance {

instance_type create(const std::set<std::string> &layers, const std::set<std::string> &extensions) {
	VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL, 0,
		NULL};
	create_info.enabledLayerCount = (uint32_t) layers.size();
	const std::vector<const char *> layers_pointers(util::to_pointers(layers));
	create_info.ppEnabledLayerNames = layers_pointers.empty() ? NULL : &layers_pointers.front();
	const std::vector<const char *> extensions_pointers(util::to_pointers(extensions));
	create_info.enabledExtensionCount = (uint32_t) extensions.size();
	create_info.ppEnabledExtensionNames = extensions_pointers.empty() ? NULL : &extensions_pointers.front();
	VkInstance instance;
	VKCHECK(vkCreateInstance(&create_info, NULL, &instance));
	return instance_type(instance);
}

}  // namespace instance
}  // namespace vcc


