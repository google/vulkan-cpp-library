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
#include <cassert>
#include <sstream>
#include <vcc/shader_module.h>

namespace vcc {
namespace shader_module {

shader_module_type create(const type::supplier<const device::device_type> &device,
		std::istream &&stream) {
	std::ostringstream ss;
	ss << stream.rdbuf();
	const std::string string(ss.str());
	VkShaderModuleCreateInfo create = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, NULL};
	create.pCode = (const uint32_t *) string.c_str();
	assert(stream);
	assert(string.size() % 4 == 0);
	create.codeSize = string.size();

	VkShaderModule shader_module;
	VKCHECK(vkCreateShaderModule(internal::get_instance(*device), &create, NULL, &shader_module));
	return shader_module_type(shader_module, device);
}

}  // namespace shader_module
}  // namespace vcc


