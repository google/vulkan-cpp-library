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
#include <vcc/descriptor_set_layout.h>

namespace vcc {
namespace descriptor_set_layout {

std::pair<std::vector<VkDescriptorSetLayoutBinding>, std::vector<std::vector<VkSampler>>> convert_bindings(const std::vector<descriptor_set_layout_binding> &bindings) {
	std::vector<VkDescriptorSetLayoutBinding> converted_bindings;
	std::vector<std::vector<VkSampler>> converted_samplers;
	converted_bindings.reserve(bindings.size());
	for (const descriptor_set_layout_binding &binding : bindings) {
		VkDescriptorSetLayoutBinding converted_binding;
		converted_binding.binding = binding.binding;
		converted_binding.descriptorType = binding.descriptorType;
		converted_binding.descriptorCount = binding.descriptorCount;
		converted_binding.stageFlags = binding.stageFlags;

		std::vector<VkSampler> samplers;
		samplers.reserve(binding.immutableSamplers.size());
		std::transform(binding.immutableSamplers.begin(), binding.immutableSamplers.end(),
			std::back_inserter(samplers), [](const type::supplier<sampler::sampler_type> &sampler) {
			return internal::get_instance(*sampler);
		});
		converted_binding.pImmutableSamplers = samplers.data();
		converted_bindings.emplace_back(converted_binding);
		converted_samplers.push_back(std::move(samplers));
	}
	return std::make_pair(std::move(converted_bindings), std::move(converted_samplers));
}

descriptor_set_layout_type create(const type::supplier<device::device_type> &device, const std::vector<descriptor_set_layout_binding> &bindings) {
	VkDescriptorSetLayoutCreateInfo create = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0};
	create.bindingCount = (uint32_t) bindings.size();
	std::vector<VkDescriptorSetLayoutBinding> converted_bindings;
	std::vector<std::vector<VkSampler>> converted_samplers;
	std::tie(converted_bindings, converted_samplers) = (convert_bindings(bindings));
	create.pBindings = converted_bindings.data();
	VkDescriptorSetLayout layout;
	VKCHECK(vkCreateDescriptorSetLayout(internal::get_instance(*device),
		&create, NULL, &layout));
	return descriptor_set_layout_type(layout, device);
}

}  // namespace descriptor_set_layout
}  // namespace vcc
