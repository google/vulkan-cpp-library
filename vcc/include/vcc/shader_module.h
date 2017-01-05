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
#ifndef SHADER_MODULE_H_
#define SHADER_MODULE_H_

#include <vcc/device.h>

namespace vcc {
namespace shader_module {

struct shader_module_type : internal::movable_destructible_with_parent<VkShaderModule,
		const device::device_type, vkDestroyShaderModule> {

	friend VCC_LIBRARY shader_module_type create(const type::supplier<const device::device_type> &,
		std::istream &&);

	shader_module_type() = default;
	shader_module_type(shader_module_type &&) = default;
	shader_module_type(const shader_module_type &) = delete;
	shader_module_type &operator=(shader_module_type &&) = default;
	shader_module_type &operator=(const shader_module_type &) = delete;

private:
	shader_module_type(VkShaderModule instance,
		const type::supplier<const device::device_type> &parent)
		: movable_destructible_with_parent(instance, parent) {}
};

VCC_LIBRARY shader_module_type create(const type::supplier<const device::device_type> &device,
	std::istream &&stream);

}  // namespace shader_module
}  // namespace vcc


#endif /* SHADER_MODULE_H_ */
