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
#ifndef DESCRIPTOR_SET_LAYOUT_H_
#define DESCRIPTOR_SET_LAYOUT_H_

#include <vcc/device.h>
#include <vcc/sampler.h>

namespace vcc {
namespace descriptor_set_layout {

struct descriptor_set_layout_binding {
	uint32_t binding;
	VkDescriptorType descriptorType;
	uint32_t descriptorCount;
	VkShaderStageFlags stageFlags;
	std::vector<type::supplier<const sampler::sampler_type>> immutableSamplers;
};

struct descriptor_set_layout_type : internal::movable_destructible_with_parent<
		VkDescriptorSetLayout, const device::device_type, vkDestroyDescriptorSetLayout> {
	friend VCC_LIBRARY descriptor_set_layout_type create(
		const type::supplier<const device::device_type> &device,
		const std::vector<descriptor_set_layout_binding> &bindings);

	descriptor_set_layout_type() = default;
	descriptor_set_layout_type(descriptor_set_layout_type &&) = default;
	descriptor_set_layout_type(const descriptor_set_layout_type &) = delete;
	descriptor_set_layout_type &operator=(descriptor_set_layout_type &&) = default;
	descriptor_set_layout_type &operator=(const descriptor_set_layout_type &) = delete;

private:
	descriptor_set_layout_type(VkDescriptorSetLayout instance,
		const type::supplier<const device::device_type> &parent)
		: movable_destructible_with_parent(instance, parent) {}
};

VCC_LIBRARY descriptor_set_layout_type create(
		const type::supplier<const device::device_type> &device,
		const std::vector<descriptor_set_layout_binding> &bindings);

}  // namespace descriptor_set_layout
}  // namespace vcc


#endif /* DESCRIPTOR_SET_LAYOUT_H_ */
