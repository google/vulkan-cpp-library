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

std::vector<VkDescriptorSetLayoutBinding> convert_bindings(const std::vector<descriptor_set_layout_binding> &bindings) {
	std::vector<VkDescriptorSetLayoutBinding> converted_bindings;
	converted_bindings.reserve(bindings.size());
	std::transform(bindings.begin(), bindings.end(), std::back_inserter(converted_bindings),
			[](const descriptor_set_layout_binding &binding) {
		VkDescriptorSetLayoutBinding converted_binding;
		converted_binding.binding = binding.binding;
		converted_binding.descriptorType = binding.descriptorType;
		converted_binding.descriptorCount = binding.descriptorCount;
		converted_binding.stageFlags = binding.stageFlags;

		// TODO(gardell): A bit of a C-style hack.
		/*VkSampler *mutable_samplers = new VkSampler[binding.immutableSamplers.size()];
		std::transform(binding.immutableSamplers.begin(), binding.immutableSamplers.end(),
				mutable_samplers, [](const type::supplier<sampler::sampler_type> &sampler){
			return sampler->instance;
		});
		converted_binding.pImmutableSamplers = mutable_samplers;*/
		converted_binding.pImmutableSamplers = NULL;
		return converted_binding;
	});
	return std::move(converted_bindings);
}

descriptor_set_layout_type create(const type::supplier<device::device_type> &device, const std::vector<descriptor_set_layout_binding> &bindings) {
	VkDescriptorSetLayoutCreateInfo create = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0};
	create.bindingCount = (uint32_t) bindings.size();
	const std::vector<VkDescriptorSetLayoutBinding> converted_bindings(convert_bindings(bindings));
	create.pBindings = converted_bindings.data();
	VkDescriptorSetLayout layout;
	VKCHECK(vkCreateDescriptorSetLayout(internal::get_instance(*device),
		&create, NULL, &layout));

	// TODO(gardell): A bit of a C-style hack.
	/*for (const VkDescriptorSetLayoutBinding &binding : converted_bindings) {
		delete[] binding.pImmutableSamplers;
	}*/
	return descriptor_set_layout_type(layout, device);
}

}  // namespace descriptor_set_layout
}  // namespace vcc
